#include "ctp_md_engine.h"
#include <unistd.h>


class quot_consumer : public md_event_listener
{
public:
	void handle_quot (quot_t* quot_ptr);
};

void quot_consumer::handle_quot (quot_t* quot_ptr)
{
	pr_debug ("bid_price: %f, ask_price: %f, bid_qty: %d, ask_qty: %d\n",
		quot_ptr->bid_price, quot_ptr->ask_price, quot_ptr->bid_qty, quot_ptr->ask_qty);
}

int
main ()
{
	i_md_engine* engine_ptr = new ctp_md_engine;
	auto quot_consumer_ptr = new quot_consumer;
	quot_consumer_ptr->listener_name = std::string ("quot_consumer");
	engine_ptr->register_md_event_listener (quot_consumer_ptr);
	std::vector<std::string> contract_vec;
	contract_vec.push_back (std::string ("rb1901"));
	engine_ptr->init (std::string ("9999"), std::string ("032862"), 
		std::string ("evergeen"), std::string ("tcp://180.168.146.187:10013"));
		/// std::string ("evergeen"), std::string ("tcp://180.168.146.187:10031"));
	engine_ptr->start ();
	engine_ptr->subscribe_md (contract_vec);

	printf ("start ok.\n");
	for (;;) {
		sleep (100000);
	}
}
