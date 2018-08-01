#include "ctp_trade_engine.h"

void ctp_trade_engine::init ()
{
	order_rsp_ptr = new order_rsp_info_t;
    cancel_rsp_ptr = new cancel_rsp_info_t;
    order_match_ptr = new order_match_info_t;
}

/*
 * SPI functions
 */
void ctp_trade_engine::OnFrontConnected()
{
    pr_info ("[OnFrontConnected] (idx)" << curAccountIdx);
    account_units[curAccountIdx].connected = true;
}

void ctp_trade_engine::OnFrontDisconnected(int nReason)
{
    pr_info ("[OnFrontDisconnected] reason=" << nReason);
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
        KF_LOG_ERROR(logger, "[OnRspAuthenticate]" << " (errId)" << pRspInfo->ErrorID
                                                   << " (errMsg)" << GBK2UTF8(pRspInfo->ErrorMsg));
    }
    else
    {
        pr_info ("[OnRspAuthenticate]" << " (userId)" <<  pRspAuthenticateField->UserID
                                                  << " (brokerId)" << pRspAuthenticateField->BrokerID
                                                  << " (product)" << pRspAuthenticateField->UserProductInfo
                                                  << " (rid)" << nRequestID);
        for (auto& unit: account_units)
        {
            if (unit.auth_rid == nRequestID)
                unit.authenticated = true;
        }
    }
}

void ctp_trade_engine::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo,
                                 int nRequestID, bool bIsLast)
{
    if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
    {
        KF_LOG_ERROR(logger, "[OnRspUserLogin]" << " (errId)" << pRspInfo->ErrorID
                                                << " (errMsg)" << GBK2UTF8(pRspInfo->ErrorMsg));
    }
    else
    {
        pr_info ("[OnRspUserLogin]" << " (Bid)" << pRspUserLogin->BrokerID
                                               << " (Uid)" << pRspUserLogin->UserID
                                               << " (maxRef)" << pRspUserLogin->MaxOrderRef
                                               << " (Fid)" << pRspUserLogin->FrontID
                                               << " (Sid)" << pRspUserLogin->SessionID);
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
        KF_LOG_ERROR(logger, "[OnRspSettlementInfoConfirm]" << " (errId)" << pRspInfo->ErrorID
                                                            << " (errMsg)" << GBK2UTF8(pRspInfo->ErrorMsg));
    }
    else
    {
        pr_info ("[OnRspSettlementInfoConfirm]" << " (brokerID)" << pSettlementInfoConfirm->BrokerID
                                                           << " (investorID)" << pSettlementInfoConfirm->InvestorID
                                                           << " (confirmDate)" << pSettlementInfoConfirm->ConfirmDate
                                                           << " (confirmTime)" << pSettlementInfoConfirm->ConfirmTime);
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
        KF_LOG_ERROR(logger, "[OnRspUserLogout]" << " (errId)" << pRspInfo->ErrorID
                                                 << " (errMsg)" << GBK2UTF8(pRspInfo->ErrorMsg));
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
    raw_writer->write_error_frame(pInputOrderAction, sizeof(CThostFtdcInputOrderActionField), source_id, MSG_TYPE_LF_ORDER_ACTION_CTP, bIsLast, nRequestID, errorId, errorMsg);
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
    raw_writer->write_error_frame(pInvestorPosition, sizeof(CThostFtdcInvestorPositionField), source_id, MSG_TYPE_LF_RSP_POS_CTP, bIsLast, nRequestID, errorId, errorMsg);
}

void ctp_trade_engine::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    auto rtn_order = parseFrom(*pOrder);
    on_rtn_order(&rtn_order);
    raw_writer->write_frame(pOrder, sizeof(CThostFtdcOrderField),
                            source_id, MSG_TYPE_LF_RTN_ORDER_CTP,
                            1/*islast*/, (pOrder->RequestID > 0) ? pOrder->RequestID: -1);
}

void ctp_trade_engine::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    auto rtn_trade = parseFrom(*pTrade);
    on_rtn_trade(&rtn_trade);
    raw_writer->write_frame(pTrade, sizeof(CThostFtdcTradeField),
                            source_id, MSG_TYPE_LF_RTN_TRADE_CTP, 1/*islast*/, -1/*invalidRid*/);
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
    raw_writer->write_error_frame(pTradingAccount, sizeof(CThostFtdcTradingAccountField), source_id, MSG_TYPE_LF_RSP_ACCOUNT_CTP, bIsLast, nRequestID, errorId, errorMsg);
}
