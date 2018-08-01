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
		long start_time = get_nanosec ();
		while (!connected && get_nanosec () - start_time < timeout_nsec)
		{}
	}
}

void ctp_md_engine::subscribe_md (std::vector<std::string> contract_vec)
{

}

void ctp_md_engine::subscribe_l2_md (std::vector<std::string> contract_vec)
{

}
