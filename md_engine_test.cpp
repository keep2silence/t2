#include "ctp_md_engine.h"
#include <unistd.h>

int
main ()
{
	i_md_engine* engine_ptr = new ctp_md_engine;
	std::vector<std::string> contract_vec;
	contract_vec.push_back (std::string ("rb1901"));
	engine_ptr->init (std::string ("9999"), std::string ("032862"), 
		std::string ("evergeen"), std::string ("tcp://180.168.146.187:10031"));
	engine_ptr->start ();
	engine_ptr->subscribe_md (contract_vec);

	printf ("start ok.\n");
	sleep (10000);
}
