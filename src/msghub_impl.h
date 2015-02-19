#ifndef _MSGHUB_MSGHUB_IMPL_H_
#define _MSGHUB_MSGHUB_IMPL_H_

#include "hub.h"
#include "msghub.h"
#include "hubconnection.h"
#include "hubclient.h"
#include "hubmessage.h"

#include <memory>
#include <cstdlib>
#include <functional>
#include <algorithm>
#include <vector>

#include <vector>
#include <map>
#include <set>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>

#include <boost/thread/thread.hpp>
#include <boost/lexical_cast.hpp>

using boost::asio::ip::tcp;

class msghub_impl : public hub
{
public:

private:
	typedef std::map < std::string, msghub::onmessage >::iterator messagemapit;
	std::map < std::string, msghub::onmessage > messagemap_;

public:

private:
	bool		initok_;
	std::vector<boost::shared_ptr<boost::thread> > threads_;
	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;

	boost::shared_ptr<hubconnection> publisher_;

	void initpool(uint8_t threads);

public:

	msghub_impl(boost::asio::io_service& io_service);
	~msghub_impl();
	bool connect(const std::string& hostip, uint16_t port, uint8_t threads = 1);
	bool create(uint32_t port, uint8_t threads = 1);
	void join();
	bool publish(const std::string& topic, const std::vector<char>& message);
	bool publish(const std::string& topic, const std::string& message);
	bool unsubscribe(const std::string& topic);
	bool subscribe(const std::string& topic, msghub::onmessage handler);

public:

	typedef std::set<boost::shared_ptr<hubclient>> subscriberset;
	typedef std::map<std::string, subscriberset>::iterator subscribersit;
	std::map<std::string, subscriberset> subscribers_;
	boost::mutex subscriberslock_;

	void distribute(boost::shared_ptr<hubclient> subscriber, hubmessage& msg);
	void deliver(boost::shared_ptr<hubconnection> publisher, hubmessage& msg);
	void accept_next();
	void handle_accept(boost::shared_ptr<hubclient> session, const boost::system::error_code& error);
};

#endif