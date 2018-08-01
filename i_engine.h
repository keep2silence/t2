#ifndef __I_ENGINE_H__
#define __I_ENGINE_H__

class i_engine
{
public:
	virtual void init() = 0;
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
    virtual string name() const = 0;
};

#endif
