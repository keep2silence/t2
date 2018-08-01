#ifndef __I_MD_ENGINE_H__
#define __I_MD_ENGINE_H__

#include <vector>
#include <string>
#include "base.h"

class quot_t;

struct md_event_listener 
{
	void handle_quot (quot_t* quot_ptr) = 0;
	std::string listener_name;
}

class i_md_engine 
{
public:
	virtual ~i_md_engine ()
	{}

	virtual void init (void *) = 0;
	virtual void start () = 0;
	virtual void subscribe_md (std::vector<std::string> contract_vec) = 0;
	virtual void subscribe_l2_md (std::vector<std::string> contract_vec) = 0;
	

	bool register_md_event_listener (md_event_listener* listener_ptr)
	{
		if (listener_ptr == nullptr) {
			return false;
		}

		for (size_t i = 0; i < md_event_listener_vec.size (); ++i) {
			if (listener_ptr == md_event_listener_vec[i]) {
				pr_error ("listener: %s had been registered.\n", 
					listener_ptr->listener_name.c_str ());
				return false;
			}
		}

		md_event_listener_vec.push_back (listener_ptr);
		pr_info ("listener: %s register ok.\n",
				listener_ptr->listener_name.c_str ());
	}

private:
	std::vector<md_event_listener *> md_event_listener_vec;
};

#endif
