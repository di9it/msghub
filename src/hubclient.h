#ifndef _MSGHUB_HUBCLIENT_H_
#define _MSGHUB_HUBCLIENT_H_

#include "hub.h"
//#include "msghub.h"
#include "hubmessage.h"

#include <memory>
#include <cstdlib>
#include <functional>
#include <algorithm>
#include <vector>
#include <deque>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include <boost/thread/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>

using boost::asio::ip::tcp;

class hubclient : public boost::enable_shared_from_this<hubclient>
{
public:
	hubclient(boost::asio::io_service& io_service, hub& proc);
	tcp::socket& socket();
	void start();
	void write(const hubmessage& msg);
	void handle_read_header(const boost::system::error_code& error);
	void handle_read_body(const boost::system::error_code& error);
	void handle_write(const boost::system::error_code& error);

private:
	boost::mutex		write_msgs_lock_;
	tcp::socket			socket_;
	hub&				distributor_;
	hubmessage			inmsg_;
	hubmessage_queue	outmsg_queue_;
};

#endif