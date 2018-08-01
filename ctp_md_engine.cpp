#include "ctp_md_engine.h"

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

void ctp_md_engine::subscribe_md (std::vector<std::string> contract_vec)
{

}

void ctp_md_engine::subscribe_l2_md (std::vector<std::string> contract_vec)
{

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
            pr_error ("[request] logout failed!" << " (Brokerid: %s, uid: %s",
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

void ctp_md_engine::subscribeMarketData(const vector<string>& instruments, const vector<string>& markets)
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
        pr_error ("[OnRspUserLogin]" << " (errID: %d), errmsg: %s",
			pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }
    else
    {
        pr_info ("[OnRspUserLogin]" << " (Brokerid: %s), uid: %s, sname: %s\n",
			 pRspUserLogin->BrokerID, pRspUserLogin->UserID, pRspUserLogin->SystemName);
        logged_in = true;
    }
}

void ctp_md_engine::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
    {
        pr_error ("[OnRspUserLogout]" << " (errID: %d), errmsg: %s\n", 
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
    auto data = parseFrom(*pDepthMarketData);
    on_market_data(&data);
}

