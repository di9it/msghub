#ifndef _MSGHUB_HUB_H_
#define _MSGHUB_HUB_H_

#include <memory>

class hubclient;
class hubconnection;
class hubmessage;

class hub
{
public:
	virtual void distribute(std::shared_ptr<hubclient> const& subscriber, hubmessage const& msg) = 0;
	virtual void deliver(hubmessage const& msg) = 0;
};
#endif
