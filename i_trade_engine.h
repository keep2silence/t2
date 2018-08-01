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
    virtual void handle_match_rsp (order_match_info* order_match_info_ptr) = 0;
};

class i_trade_engine 
{
public:
	virtual ~i_trade_engine ()
	{}

	virtual void init (void *) = 0;
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

private:
	std::vector<trade_event_listener *> trade_event_listener_vec;
};

class i_trade_engine 
{

};

#endif
