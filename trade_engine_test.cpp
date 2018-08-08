#include "ctp_trade_engine.h"
#include <unistd.h>


class trade_event_consumer : public trade_event_listener
{
public:
	virtual void handle_order_rsp (order_rsp_info_t* order_rsp_info_ptr);
    virtual void handle_cancel_rsp (cancel_rsp_info_t* cancel_rsp_info_ptr);
    virtual void handle_match_rsp (order_match_info_t* order_match_info_ptr);
};

void trade_event_consumer::handle_order_rsp (order_rsp_info_t* order_rsp_ptr)
{
	pr_debug ("handle_order_rsp, order_id: %d.\n", order_rsp_ptr->order_id);
}

void trade_event_consumer::handle_cancel_rsp (cancel_rsp_info_t* cancel_rsp_ptr)
{
	pr_debug ("handle_cancel_rsp, order_id: %d.\n", cancel_rsp_ptr->order_id);
}

void trade_event_consumer::handle_match_rsp (order_match_info_t* match_rsp_ptr)
{
	pr_debug ("handle_match_rsp, order_id: %d.\n", match_rsp_ptr->order_id);
}

int
main ()
{
	i_trade_engine* engine_ptr = new ctp_trade_engine;
	auto trade_event_consumer_ptr = new trade_event_consumer;
	trade_event_consumer_ptr->listener_name = std::string ("trade_consumer");
	engine_ptr->register_trade_event_listener (trade_event_consumer_ptr);
	std::vector<std::string> contract_vec;
	contract_vec.push_back (std::string ("rb1901"));

	trade_engine_ctx te_ctx;
	/// te_ctx.front_uri = std::string ("tcp://180.168.146.187:10030");
	te_ctx.front_uri = std::string ("tcp://180.168.146.187:10003");
	engine_ptr->init (&te_ctx);

	printf ("start ok.\n");
	for (;;) {
		sleep (100000);
	}
}
