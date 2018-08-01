#ifndef __CTP_TRADE_ENGINE_H__
#define __CTP_TRADE_ENGINE_H__

#include <string>
#include "ThostFtdcTraderApi.h"

/**
 * account information unit extra for CTP is here.
 */
struct account_unit_ctp
{
    /// api
    CThostFtdcTraderApi* api;
    /// extra information
    std::string  auth_code;
    int     front_id;
    int     session_id;
    /// internal flags
    bool    initialized;
    bool    connected;
    bool    authenticated;
    bool    settle_confirmed;
    bool    logged_in;
    /// some rids
    int     auth_rid;
    int     settle_rid;
    int     login_rid;
};


class ctp_trade_engine : public i_trade_engine, public CThostFtdcTraderSpi
{
public:
	




    // from config
    string front_uri;
    bool need_settleConfirm;
    bool need_authenticate;
    int curAccountIdx;
    vector<AccountUnitCTP> account_units;

public:
    // SPI
    ///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
    virtual void OnFrontConnected();

    ///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
    ///@param nReason 错误原因
    ///        0x1001 网络读失败
    ///        0x1002 网络写失败
    ///        0x2001 接收心跳超时
    ///        0x2002 发送心跳失败
    ///        0x2003 收到错误报文
    virtual void OnFrontDisconnected(int nReason);

    ///客户端认证响应
    virtual void OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///登录请求响应
    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, 
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///投资者结算结果确认响应
    virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///登出请求响应
    virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, 
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///报单录入请求响应 (cjiang: this only be called when there is error)
    virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, 
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///报单操作请求响应
    virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///请求查询投资者持仓响应
    virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///报单通知
    virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);

    ///成交通知
    virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);

    ///请求查询资金账户响应
    virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, 
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

};

#endif