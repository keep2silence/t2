#ifndef __I_MD_ENGINE_H__
#define __I_MD_ENGINE_H__

#include <vector>
#include <string>
#include "base.h"
#include "quot.h"

struct md_event_listener 
{
	virtual ~md_event_listener ()
	{}
	virtual void handle_quot (quot_t* quot_ptr) = 0;
	std::string listener_name;
};

class i_md_engine
{
public:
	virtual ~i_md_engine ()
	{}

	virtual void init(std::string bid, std::string uid, std::string pass, std::string uri) = 0;
	virtual void start () = 0;
	virtual void stop () = 0;

    /** use api to connect to front */
    virtual void connect(long timeout_nsec) = 0;
    /** use api to log in account */
    virtual void login(long timeout_nsec) = 0;
    /** use api to log out */
    virtual void logout() = 0;
    /** release api*/
    virtual void release_api() = 0;
    /** return true if engine connected to server */
    virtual bool is_connected() const = 0;
    /** return true if all accounts have been logged in */
    virtual bool is_logged_in() const = 0;
    /** get engine's name */
    virtual std::string name() const = 0;


	virtual void subscribe_md (const std::vector<std::string>&) = 0;

	virtual bool register_md_event_listener (md_event_listener* listener_ptr) = 0;
};

#endif
