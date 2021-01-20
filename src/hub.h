#ifndef _MSGHUB_HUB_H_
#define _MSGHUB_HUB_H_

#include <boost/shared_ptr.hpp>

class hubclient;
class hubconnection;
class hubmessage;

class hub
{
public:
	virtual void distribute(boost::shared_ptr<hubclient> subscriber, hubmessage& msg) = 0;
	virtual void deliver(hubmessage& msg) = 0;
};
#endif
