#include "ctp_md_engine.h"
#include "time_calc.h"
#include <string.h>

void ctp_md_engine::init (void *)
{

}

void ctp_md_engine::start () 
{

}

void ctp_md_engine::connect (long timeout_nsec)
{
	if (api == nullptr) {
		api = CThostFtdcMdApi::CreateFtdcMdApi();
		if (api == nullptr) {
			pr_emerg ("CTP_MD failed to create api");
		}
		api->RegisterSpi(this);
	}

	if (connected == false) {
		api->RegisterFront((char*)front_uri.c_str());
		api->Init();
		long start_time = nanosec ();
		while (!connected && nanosec () - start_time < timeout_nsec)
		{}
	}
}

void ctp_md_engine::login (long timeout_nsec)
{
    if (!logged_in) {
        CThostFtdcReqUserLoginField req = {};
        strcpy(req.BrokerID, broker_id.c_str());
        strcpy(req.UserID, user_id.c_str());
        strcpy(req.Password, password.c_str());
        if (api->ReqUserLogin(&req, reqId++)) {
            pr_error ("[request] login failed! (Brokerid: %s, uid: %s\n",
				req.BrokerID, req.UserID);
        }
        long start_time = nanosec ();
        while (!logged_in && nanosec () - start_time < timeout_nsec)
        {}
    }
}

void ctp_md_engine::logout()
{
    if (logged_in)
    {
        CThostFtdcUserLogoutField req = {};
        strcpy(req.BrokerID, broker_id.c_str());
        strcpy(req.UserID, user_id.c_str());
        if (api->ReqUserLogout(&req, reqId++))
        {
            pr_error ("[request] logout failed! Brokerid: %s, uid: %s",
				req.BrokerID, req.UserID);
        }
    }
    connected = false;
    logged_in = false;
}

void ctp_md_engine::release_api()
{
    if (api != nullptr)
    {
        api->Release();
        api = nullptr;
    }
}

void ctp_md_engine::subscribe_md (const std::vector<std::string>& instruments)
{
    int nCount = instruments.size();
    char* insts[nCount];
    for (int i = 0; i < nCount; i++)
        insts[i] = (char*)instruments[i].c_str();
    api->SubscribeMarketData(insts, nCount);
}

/*
 * SPI functions
 */
void ctp_md_engine::OnFrontConnected()
{
    pr_info ("[OnFrontConnected]");
    connected = true;
}

void ctp_md_engine::OnFrontDisconnected(int nReason)
{
    pr_info ("[OnFrontDisconnected] reason=%d", nReason);
    connected = false;
    logged_in = false;
}

#define GBK2UTF8(msg) msg

void ctp_md_engine::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
    {
        pr_error ("[OnRspUserLogin] (errID: %d), errmsg: %s",
			pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }
    else
    {
        pr_info ("[OnRspUserLogin] (Brokerid: %s), uid: %s, sname: %s\n",
			 pRspUserLogin->BrokerID, pRspUserLogin->UserID, pRspUserLogin->SystemName);
        logged_in = true;
    }
}

void ctp_md_engine::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
    {
        pr_error ("[OnRspUserLogout] (errID: %d), errmsg: %s\n", 
			pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }
    else
    {
        pr_info ("[OnRspUserLogout] (Brokerid: %s, uid: %s\n", 
			pUserLogout->BrokerID, pUserLogout->UserID);
        logged_in = false;
    }
}

void ctp_md_engine::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
    {
        pr_error ("[OnRspSubMarketData] (errID): %d, errmsg: %s, InstrumentID: %s\n", 
			pRspInfo->ErrorID, pRspInfo->ErrorMsg,
            ((pSpecificInstrument != nullptr) ?  pSpecificInstrument->InstrumentID : "null"));
    }
}

void ctp_md_engine::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	myquot.bid_price = pDepthMarketData->BidPrice1;
	myquot.ask_price = pDepthMarketData->AskPrice1;
	myquot.bid_qty   = pDepthMarketData->BidVolume1;
	myquot.ask_qty   = pDepthMarketData->AskVolume1;
#ifdef __USE_LEVEL2__
	myquot.bid_price_array[0] = pDepthMarketData->BidPrice2;
	myquot.ask_price_array[0] = pDepthMarketData->AskPrice2;
	myquot.bid_qty_array[0]   = pDepthMarketData->BidVolume2;
	myquot.ask_qty_array[0]   = pDepthMarketData->AskVolume2;
	
	myquot.bid_price_array[1] = pDepthMarketData->BidPrice3;
	myquot.ask_price_array[1] = pDepthMarketData->AskPrice3;
	myquot.bid_qty_array[1]   = pDepthMarketData->BidVolume3;
	myquot.ask_qty_array[1]   = pDepthMarketData->AskVolume3;

	myquot.bid_price_array[2] = pDepthMarketData->BidPrice4;
	myquot.ask_price_array[2] = pDepthMarketData->AskPrice4;
	myquot.bid_qty_array[2]   = pDepthMarketData->BidVolume4;
	myquot.ask_qty_array[2]   = pDepthMarketData->AskVolume4;

	myquot.bid_price_array[3] = pDepthMarketData->BidPrice5;
	myquot.ask_price_array[3] = pDepthMarketData->AskPrice5;
	myquot.bid_qty_array[3]   = pDepthMarketData->BidVolume5;
	myquot.ask_qty_array[3]   = pDepthMarketData->AskVolume5;
#endif
	myquot.last_price         = pDepthMarketData->LastPrice;
	myquot.tuneover           = pDepthMarketData->Turnover;
	myquot.volume             = pDepthMarketData->Volume;
	myquot.open_interest      = pDepthMarketData->OpenInterest;
	myquot.upper_limit        = pDepthMarketData->UpperLimitPrice;
	myquot.lower_limit        = pDepthMarketData->LowerLimitPrice;
	myquot.trade_date         = atoi (pDepthMarketData->TradingDay);
	memcpy (myquot.contract_name, pDepthMarketData->InstrumentID, 31);
	
	for (size_t i = 0; i < md_event_listener_vec.size (); ++i) {
		md_event_listener_vec[0]->handle_quot (&myquot);
	}
}

