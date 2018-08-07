#include "ctp_trade_engine.h"
#include "time_calc.h"
#include <string.h>
#include <strings.h>

void ctp_trade_engine::init (trade_engine_ctx* ctx_ptr)
{
	front_uri = ctx_ptr->front_uri;

	bzero (&order_req, sizeof (order_req));

	bzero (&order_action_req, sizeof (order_action_req));

	need_authenticate = true;
	order_rsp_ptr = new order_rsp_info_t;
    cancel_rsp_ptr = new cancel_rsp_info_t;
    order_match_ptr = new order_match_info_t;
}

void ctp_trade_engine::resize_accounts(int account_num)
{
    account_units.resize (account_num);
	trade_accounts.resize (account_num);
}

static void split_to_vector (std::string& line, std::vector<std::string> &stdvec, char sep)
{
    stdvec.clear ();
    std::string::size_type pos;
    while ((pos = line.find_first_of (sep)) != std::string::npos) {
        std::string str (line.substr (0, pos));
        stdvec.push_back (str);
        line = line.substr (pos + 1);
    }
    stdvec.push_back (line);
}

void ctp_trade_engine::load_account(int idx, std::string user_config_str)
{
	std::vector<std::string> user_config_vec;
	split_to_vector (user_config_str, user_config_vec, ',');
    // internal load
    std::string broker_id = user_config_vec[0];
    std::string user_id = user_config_vec[1];
    std::string investor_id = user_config_vec[2];
    std::string password = user_config_vec[3];

    AccountUnitCTP& unit = account_units[idx];
    unit.api = nullptr;
    unit.front_id = -1;
    unit.session_id = -1;
    unit.initialized = false;
    unit.connected = false;
    unit.authenticated = false;
    unit.logged_in = false;
    unit.settle_confirmed = false;
    if (need_authenticate)
        unit.auth_code = user_config_vec[4];

    // set up
    TradeAccount& account = trade_accounts[idx];
    strncpy(account.BrokerID, broker_id.c_str(), 19);
    strncpy(account.InvestorID, investor_id.c_str(), 19);
    strncpy(account.UserID, user_id.c_str(), 16);
    strncpy(account.Password, password.c_str(), 21);
}

void ctp_trade_engine::connect(long timeout_nsec)
{
    for (size_t idx = 0; idx < account_units.size(); idx ++)
    {
        AccountUnitCTP& unit = account_units[idx];
        if (unit.api == nullptr)
        {
            CThostFtdcTraderApi* api = CThostFtdcTraderApi::CreateFtdcTraderApi();
            if (!api)
            {
                pr_error ("CTP_TD failed to create api");
            }
            api->RegisterSpi(this);
            unit.api = api;
        }
        if (!unit.connected)
        {
            curAccountIdx = idx;
            unit.api->RegisterFront((char*)front_uri.c_str());
            unit.api->SubscribePublicTopic(THOST_TERT_QUICK); // need check
            unit.api->SubscribePrivateTopic(THOST_TERT_QUICK); // need check
            if (!unit.initialized)
            {
                unit.api->Init();
                unit.initialized = true;
            }
            long start_time = nanosec ();
            while (!unit.connected && nanosec () - start_time < timeout_nsec)
            {}
        }
    }
}

void ctp_trade_engine::login(long timeout_nsec)
{
    for (size_t idx = 0; idx < account_units.size(); idx ++)
    {
        AccountUnitCTP& unit = account_units[idx];
        TradeAccount& account = accounts[idx];
        // authenticate
        if (need_authenticate && !unit.authenticated)
        {
            struct CThostFtdcReqAuthenticateField req = {};
            strcpy(req.BrokerID, account.BrokerID);
            strcpy(req.UserID, account.UserID);
            strcpy(req.AuthCode, unit.auth_code.c_str());
            unit.auth_rid = request_id;
            if (unit.api->ReqAuthenticate(&req, request_id++))
            {
                pr_error ("[request] auth failed!(Broker_id: %s), uid: %s, auth: %s\n",
					req.BrokerID, req.UserID, req.AuthCode);
            }
            long start_time = nanosec ();
            while (!unit.authenticated && nanosec () - start_time < timeout_nsec)
            {}
        }
        // login
        if (!unit.logged_in)
        {
            struct CThostFtdcReqUserLoginField req = {};
            strcpy(req.TradingDay, "");
            strcpy(req.UserID, account.UserID);
            strcpy(req.BrokerID, account.BrokerID);
            strcpy(req.Password, account.Password);
            unit.login_rid = request_id;
            if (unit.api->ReqUserLogin(&req, request_id++))
            {
                pr_error ("[request] login failed! broker_id: %s, uid: %s\n",
					 req.BrokerID, req.UserID);
            }
            long start_time = nanosec ();
            while (!unit.logged_in && nanosec () - start_time < timeout_nsec)
            {}
        }
        // confirm settlement
        if (need_settleConfirm && !unit.settle_confirmed)
        {
            struct CThostFtdcSettlementInfoConfirmField req = {};
            strcpy(req.BrokerID, account.BrokerID);
            strcpy(req.InvestorID, account.InvestorID);
            unit.settle_rid = request_id;
            if (unit.api->ReqSettlementInfoConfirm(&req, request_id++))
            {
                pr_error ("[request] settlement info failed!(Broker_id: %s), Investorid: %s\n",
					 req.BrokerID, req.InvestorID);
            }
            long start_time = nanosec ();
            while (!unit.settle_confirmed && nanosec () - start_time < timeout_nsec)
            {}
        }
    }
}

void ctp_trade_engine::logout()
{
    for (size_t idx = 0; idx < account_units.size(); idx++)
    {
        AccountUnitCTP& unit = account_units[idx];
        TradeAccount& account = accounts[idx];
        if (unit.logged_in)
        {
            CThostFtdcUserLogoutField req = {};
            strcpy(req.BrokerID, account.BrokerID);
            strcpy(req.UserID, account.UserID);
            unit.login_rid = request_id;
            if (unit.api->ReqUserLogout(&req, request_id++))
            {
                pr_error ("[request] logout failed!(broker_id: %s), uid: %s",
					req.BrokerID, req.UserID);
            }
        }
        unit.authenticated = false;
        unit.settle_confirmed = false;
        unit.logged_in = false;
    }
}

void ctp_trade_engine::release_api()
{
    for (auto& unit: account_units)
    {
        if (unit.api != nullptr)
        {
            unit.api->Release();
            unit.api = nullptr;
        }
        unit.initialized = false;
        unit.connected = false;
        unit.authenticated = false;
        unit.settle_confirmed = false;
        unit.logged_in = false;
        unit.api = nullptr;
    }
}

bool ctp_trade_engine::is_logged_in() const
{
    for (auto& unit: account_units)
    {
        if (!unit.logged_in || (need_settleConfirm && !unit.settle_confirmed))
            return false;
    }
    return true;
}

bool ctp_trade_engine::is_connected() const
{
    for (auto& unit: account_units)
    {
        if (!unit.connected)
            return false;
    }
    return true;
}

#if 0
/**
 * req functions
 */
void ctp_trade_engine::req_investor_position(const LFQryPositionField* data, int account_index, int requestId)
{
    struct CThostFtdcQryInvestorPositionField req = parseTo(*data);
    pr_debug ("[req_pos](Broker_id: %d, investorid: %d, Instrumentid: %d)",
		req.BrokerID, req.InvestorID, req.InstrumentID);

    if (account_units[account_index].api->ReqQryInvestorPosition(&req, requestId))
    {
        pr_error ("[request] investor position failed! (rid: %d), account_idx: %d", 
			requestId, account_index);
    }
}

void ctp_trade_engine::req_qry_account(const LFQryAccountField *data, int account_index, int requestId)
{
    struct CThostFtdcQryTradingAccountField req = parseTo(*data);
    pr_debug ("[req_account]" << " (Bid)" << req.BrokerID
                                         << " (Iid)" << req.InvestorID);

    if (account_units[account_index].api->ReqQryTradingAccount(&req, requestId))
    {
        pr_error ("[request] account info failed!" << " (rid)" << requestId
                                                              << " (idx)" << account_index);
    }
}
#endif

void ctp_trade_engine::req_order_insert(const order_t* order_ptr, 
	int account_index, int requestId, long rcv_time)
{
	TradeAccount& ta = trade_accounts[account_index];
    memcpy (order_req.BrokerID, ta.BrokerID, 11);
    memcpy (order_req.UserID, ta.UserID, 16);
    memcpy (order_req.InvestorID, ta.InvestorID, 13);
    memcpy (order_req.BusinessUnit, ta.BusinessUnit, 21);
    memcpy (order_req.ExchangeID, ta.ExchangeID, 9);
    memcpy (order_req.InstrumentID, ta.InstrumentID, 31);
    memcpy (order_req.OrderRef, ta.OrderRef, 13);

    order_req.Direction = order_ptr->direction;
    order_req.OffsetFlag = order_ptr->offset;
    order_req.HedgeFlag = THOST_FTDC_HF_Speculation; /// 投机

    order_req.VolumeTotalOriginal = order_ptr->order_qty;
    order_req.VolumeCondition = THOST_FTDC_VC_AV;
    order_req.MinVolume = 1;
    order_req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;

    order_req.LimitPrice = order_ptr->price / 100.0;
	if (order_ptr->order_price_type == THOST_FTDC_OPT_LimitPrice) { 
		order_req.TimeCondition = THOST_FTDC_TC_GFD;
	} else {
		/// 市价单价格为０
		order_req.TimeCondition = THOST_FTDC_TC_GFD;
	}
    order_req.OrderPriceType = order_ptr->order_price_type;

    /// order_req.StopPrice = order_ptr->StopPrice;
    /// order_req.ContingentCondition = order_ptr->ContingentCondition;

    order_req.RequestID = requestId;
    order_req.IsAutoSuspend = 0;
    order_req.UserForceClose = 0;
	
    if (account_units[account_index].api->ReqOrderInsert(&order_req, requestId))
    {
        pr_error ("[request] order insert failed (rid: %d)\n", requestId);
    }

    pr_debug ("req_order_insert requestid: %d, invectorid: %s, instrumentid: %s, order_ref: %s.\n",
		 requestId, order_req.InvestorID, order_req.InstrumentID, order_req.OrderRef);
}

void ctp_trade_engine::req_order_action(int account_index, int requestId, long rcv_time)
{
    /// struct CThostFtdcInputOrderActionField req = parseTo(*data);
    order_action_req.OrderActionRef = local_id++;
    auto& unit = account_units[account_index];
    order_action_req.FrontID = unit.front_id;
    order_action_req.SessionID = unit.session_id;

	TradeAccount& ta = trade_accounts[account_index];
    memcpy (order_action_req.BrokerID, ta.BrokerID, 11);
    memcpy (order_action_req.UserID, ta.UserID, 16);
    memcpy (order_action_req.InvestorID, ta.InvestorID, 13);
    memcpy (order_action_req.ExchangeID, ta.ExchangeID, 9);
    memcpy (order_action_req.InstrumentID, ta.InstrumentID, 31);
    memcpy (order_action_req.OrderRef, ta.OrderRef, 13);

    res.RequestID = requestId;
    res.ActionFlag = THOST_FTDC_AF_Delete;
    /// res.LimitPrice = lf.LimitPrice;

    if (unit.api->ReqOrderAction(&order_action_req, requestId))
    {
        pr_error ("[request] order action failed!(requestid: %d)", requestId);
    }
    pr_debug ("[req_order_action](requestid: %d, investorid: %s, order_ref: %s, OrderActionRef: %d)\n",
			requestId, order_action_req.InvestorID, 
			order_action_req.OrderRef, order_action_req.OrderActionRef);
}

/*
 * SPI functions
 */
void ctp_trade_engine::OnFrontConnected()
{
    pr_info ("[OnFrontConnected] (idx: %d)\n", curAccountIdx);
    account_units[curAccountIdx].connected = true;
}

void ctp_trade_engine::OnFrontDisconnected(int nReason)
{
    pr_info ("[OnFrontDisconnected] reason=%d\n", nReason);
    for (auto& unit: account_units)
    {
        unit.connected = false;
        unit.authenticated = false;
        unit.settle_confirmed = false;
        unit.logged_in = false;
    }
}

#define GBK2UTF8(msg) (msg)

void ctp_trade_engine::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField,
                                    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
    {
        pr_error ("[OnRspAuthenticate] (errId: %d), errMsg: %s\n",
			 pRspInfo->ErrorID, GBK2UTF8(pRspInfo->ErrorMsg));
    }
    else
    {
        pr_info ("[OnRspAuthenticate](userId: %s), brokerid: %s, product: %s, request_id: %d\n",
			 pRspAuthenticateField->UserID, pRspAuthenticateField->BrokerID,
             pRspAuthenticateField->UserProductInfo, nRequestID);
        for (auto& unit: account_units)
        {
            if (unit.auth_rid == nRequestID)
                unit.authenticated = true;
        }
    }
}

void ctp_trade_engine::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
    {
        pr_error ("[OnRspUserLogin](errId: %d), err_msg: %s\n",
			pRspInfo->ErrorID, GBK2UTF8(pRspInfo->ErrorMsg));
    }
    else
    {
        pr_info ("[OnRspUserLogin] (broker_id: %s), uid: %s, max_order_ref: %s, front_id: %d, session_id: %d\n",
			pRspUserLogin->BrokerID, pRspUserLogin->UserID, pRspUserLogin->MaxOrderRef,
			pRspUserLogin->FrontID, pRspUserLogin->SessionID);
        for (auto& unit: account_units)
        {
            if (unit.login_rid == nRequestID)
            {
                unit.front_id = pRspUserLogin->FrontID;
                unit.session_id = pRspUserLogin->SessionID;
                unit.logged_in = true;
            }
        }
        int max_ref = atoi(pRspUserLogin->MaxOrderRef) + 1;
        local_id = (max_ref > local_id) ? max_ref: local_id;
    }
}

void ctp_trade_engine::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
                                             CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
    {
        pr_error ("[OnRspSettlementInfoConfirm](errId: %d), err_msg: %s\n",
			pRspInfo->ErrorID, GBK2UTF8(pRspInfo->ErrorMsg));
    }
    else
    {
        pr_info ("[OnRspSettlementInfoConfirm](brokerID: %s), investorID: %s, confirmDate: %s, confirmTime: %s\n",  
			pSettlementInfoConfirm->BrokerID, pSettlementInfoConfirm->InvestorID,
            pSettlementInfoConfirm->ConfirmDate, pSettlementInfoConfirm->ConfirmTime);
        for (auto& unit: account_units)
        {
            if (unit.settle_rid == nRequestID)
            {
                unit.settle_confirmed = true;
            }
        }
    }
}

void ctp_trade_engine::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo,
                                  int nRequestID, bool bIsLast)
{
    if (pRspInfo != nullptr && pRspInfo->ErrorID == 0)
    {
        pr_error ("[OnRspUserLogout] (errId: %d), errMsg: %s\n", 
			pRspInfo->ErrorID, GBK2UTF8(pRspInfo->ErrorMsg));
    }
    else
    {
        pr_info ("[OnRspUserLogout] (brokerId: %s), userId: %s\n",
			pUserLogout->BrokerID, pUserLogout->UserID);
        for (auto& unit: account_units)
        {
            if (unit.login_rid == nRequestID)
            {
                unit.logged_in = false;
                unit.authenticated = false;
                unit.settle_confirmed = false;
            }
        }
    }
}

void ctp_trade_engine::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    int errorId = (pRspInfo == nullptr) ? 0 : pRspInfo->ErrorID;
    const char* errorMsg = (pRspInfo == nullptr) ? nullptr : pRspInfo->ErrorMsg;
    auto data = parseFrom(*pInputOrder);
    on_rsp_order_insert(&data, nRequestID, errorId, errorMsg);
	
	for (size_t i = 0; i < trade_event_listener_vec.size (); ++i) {
		trade_event_listener_vec[i]->handle_order_rsp (order_rsp_ptr);
	}
}

void ctp_trade_engine::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction,
                                   CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    int errorId = (pRspInfo == nullptr) ? 0 : pRspInfo->ErrorID;
    const char* errorMsg = (pRspInfo == nullptr) ? nullptr : EngineUtil::gbkErrorMsg2utf8(pRspInfo->ErrorMsg);
    auto data = parseFrom(*pInputOrderAction);
    on_rsp_order_action(&data, nRequestID, errorId, errorMsg);
    /// raw_writer->write_error_frame(pInputOrderAction, sizeof(CThostFtdcInputOrderActionField), source_id, MSG_TYPE_LF_ORDER_ACTION_CTP, bIsLast, nRequestID, errorId, errorMsg);
}

void ctp_trade_engine::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo,
                                     int nRequestID, bool bIsLast)
{
    int errorId = (pRspInfo == nullptr) ? 0 : pRspInfo->ErrorID;
    const char* errorMsg = (pRspInfo == nullptr) ? nullptr : EngineUtil::gbkErrorMsg2utf8(pRspInfo->ErrorMsg);
    CThostFtdcInvestorPositionField emptyCtp = {};
    if (pInvestorPosition == nullptr)
        pInvestorPosition = &emptyCtp;
    auto pos = parseFrom(*pInvestorPosition);
    on_rsp_position(&pos, bIsLast, nRequestID, errorId, errorMsg);
    /// raw_writer->write_error_frame(pInvestorPosition, sizeof(CThostFtdcInvestorPositionField), source_id, MSG_TYPE_LF_RSP_POS_CTP, bIsLast, nRequestID, errorId, errorMsg);
}

void ctp_trade_engine::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    auto rtn_order = parseFrom(*pOrder);
    on_rtn_order(&rtn_order);
    /// raw_writer->write_frame(pOrder, sizeof(CThostFtdcOrderField),
     ///                       source_id, MSG_TYPE_LF_RTN_ORDER_CTP,
        ///                    1/*islast*/, (pOrder->RequestID > 0) ? pOrder->RequestID: -1);
}

void ctp_trade_engine::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    auto rtn_trade = parseFrom(*pTrade);
    on_rtn_trade(&rtn_trade);
    /// raw_writer->write_frame(pTrade, sizeof(CThostFtdcTradeField),
       ///                     source_id, MSG_TYPE_LF_RTN_TRADE_CTP, 1/*islast*/, -1/*invalidRid*/);
}

void ctp_trade_engine::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount,
                                         CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    int errorId = (pRspInfo == nullptr) ? 0 : pRspInfo->ErrorID;
    const char* errorMsg = (pRspInfo == nullptr) ? nullptr : EngineUtil::gbkErrorMsg2utf8(pRspInfo->ErrorMsg);
    CThostFtdcTradingAccountField empty = {};
    if (pTradingAccount == nullptr)
        pTradingAccount = &empty;
    auto account = parseFrom(*pTradingAccount);
    on_rsp_account(&account, bIsLast, nRequestID, errorId, errorMsg);
    /// raw_writer->write_error_frame(pTradingAccount, sizeof(CThostFtdcTradingAccountField), source_id, MSG_TYPE_LF_RSP_ACCOUNT_CTP, bIsLast, nRequestID, errorId, errorMsg);
}
