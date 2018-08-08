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

	trade_engine_ctx te_ctx;
	/// te_ctx.front_uri = std::string ("tcp://180.168.146.187:10030");
	te_ctx.front_uri = std::string ("tcp://180.168.146.187:10003");
	snprintf (te_ctx.ta[0].BrokerID, 19, "9999");
	snprintf (te_ctx.ta[0].UserID, 16, "032862");
	snprintf (te_ctx.ta[0].Password, 21, "evergreen");
	te_ctx.max_account = 1;
	engine_ptr->init (&te_ctx);

	engine_ptr->start ();

	/// 测试下单
	engine_ptr->place_order ("i1901", d_buy, o_open, 4401, 100);

	printf ("start ok.\n");
	for (;;) {
		sleep (100000);
	}
}
