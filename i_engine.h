#ifndef __I_ENGINE_H__
#define __I_ENGINE_H__

#include <vector>
#include <string>
#include "base.h"

class quot_t;

struct md_listener 
{
	void handle_quot (quot_t* quot_ptr) = 0;
	std::string listener_name;
}

class i_md_engine 
{
public:
	virtual ~i_md_engine ()
	{}

	void init (void *) = 0;
	void start () = 0;
	
	bool register_md_listner (md_listener* listener_ptr)
	{
		if (listener_ptr == nullptr) {
			return false;
		}

		for (size_t i = 0; i < md_listener_vec.size (); ++i) {
			if (listener_ptr == md_listener_vec[i]) {
				pr_error ("listener: %s had been registered.\n", 
					listener_ptr->listener_name.c_str ());
				return false;
			}
		}
	}

private:
	std::vector<md_listener *> md_listener_vec;
};

class i_trade_engine 
{

};

#endif
