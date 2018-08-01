#ifndef __CTP_MD_ENGINE_H__
#define __CTP_MD_ENGINE_H__

#include "ThostFtdcMdApi.h"
#include <vector>
#include "i_md_engine.h"

class ctp_md_engine : public i_md_engine, public CThostFtdcMdSpi
{
public:
	ctp_md_engine ()
	{}

	virtual ~ctp_md_engine ()
	{}

	virtual void init ();
    virtual void start ();
    virtual void stop ();

    /** use api to connect to front */
    virtual void connect(long timeout_nsec);
    /** use api to log in account */
    virtual void login(long timeout_nsec);
    /** use api to log out */
    virtual void logout();
    /** release api*/
    virtual void release_api();
    /** return true if engine connected to server */
    virtual bool is_connected() const;
    /** return true if all accounts have been logged in */
    virtual bool is_logged_in() const;
    /** get engine's name */
    virtual std::string name() const;

    virtual void subscribe_md (const std::vector<std::string>& instruments);

	virtual bool register_md_event_listener (md_event_listener* listener_ptr);

private:
    std::vector<md_event_listener *> md_event_listener_vec;
    quot_t myquot;

private:
    /** ctp api */
    CThostFtdcMdApi* api;
    /** internal information */
    std::string broker_id;
    std::string user_id;
    std::string password;
    std::string front_uri;
    // internal flags
    bool connected;
    bool logged_in;
    int  reqId;

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

    ///登录请求响应
    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///登出请求响应
    virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///订阅行情应答
    virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///深度行情通知
    virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);
};

#endif
