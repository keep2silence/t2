#ifndef __I_TRADE_ENGINE_H__
#define __I_TRADE_ENGINE_H__

#include <vector>
#include <string>
#include "base.h"

class trade_event_listener
{
public:
    virtual ~trade_event_listener ()
    {}

    virtual void handle_order_rsp (order_rsp_info_t* order_rsp_info_ptr) = 0;
    virtual void handle_cancel_rsp (cancel_rsp_info_t* cancel_rsp_info_ptr) = 0;
    virtual void handle_match_rsp (order_match_info_t* order_match_info_ptr) = 0;

	std::string listener_name;
};

#if 1
/// 支持多账号同时登录
/**
 * Trade account information,
 * used when connect or login
 */
struct TradeAccount
{
    char BrokerID[19];
    char UserID[16];
    char InvestorID[19];
    char BusinessUnit[21];
    char Password[21];
    /// FeeHandlerPtr fee_handler;
};

class trade_engine_ctx
{
public:
	std::string front_uri;
};

#endif

class i_trade_engine 
{
public:
	virtual ~i_trade_engine ()
	{}

	virtual void init (trade_engine_ctx* ctx_ptr) = 0;
	virtual void start () = 0;
	
	bool register_trade_event_listner (trade_event_listener* listener_ptr)
	{
		if (listener_ptr == nullptr) {
			return false;
		}

		for (size_t i = 0; i < trade_event_listener_vec.size (); ++i) {
			if (listener_ptr == trade_event_listener_vec[i]) {
				pr_error ("listener: %s had been registered.\n", 
					listener_ptr->listener_name.c_str ());
				return false;
			}
		}

		trade_event_listener_vec.push_back (listener_ptr);
		pr_info ("listener: %s register ok.\n",
				listener_ptr->listener_name.c_str ());
	}

public:
    // config and account related
    /** resize account number */
    virtual void resize_accounts(int account_num) = 0;
    /** connect each account and api with timeout as wait interval*/
    virtual void connect(long timeout_nsec) = 0;
    /** login all account and api with timeout as wait interval */
    virtual void login(long timeout_nsec) = 0;
    /** send logout request for each api */
    virtual void logout() = 0;
    /** release api items */
    virtual void release_api() = 0;
    /** is every accounts connected? */
    virtual bool is_connected() const = 0;
    /** is every accounts logged in? */
    virtual bool is_logged_in() const = 0;

	/// 查询持仓
	virtual int query_posi (int account_id) = 0;
	
	/// 下单
	virtual int place_order (int contract_id, direction_t direction, 
		offset_flag_t offset, price_t price, int qty) = 0;

	/// 撤单
	virtual int cancel_order (int order_id) = 0;

protected:
	std::vector<trade_event_listener *> trade_event_listener_vec;
	std::vector<TradeAccount> accounts;
    /** request_id, incremental*/
    int request_id;
    /** local id, incremental */
    int local_id;
    /** current nano time */
    long cur_time;
	int account_id;

	order_rsp_info_t order_rsp;
	cancel_rsp_info_t cancel_rsp;
	order_match_info_t match_rsp;
};

#endif
