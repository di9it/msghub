#ifndef _MSGHUB_HUBCONNECTION_H_
#define _MSGHUB_HUBCONNECTION_H_

#include "hub.h"
#include "hubmessage.h"

#include <string>
#include <memory>
#include <cstdlib>
#include <functional>
#include <algorithm>
#include <vector>
#include <deque>

#include <boost/atomic.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include <boost/thread/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>

using boost::asio::ip::tcp;

class hubconnection : public boost::enable_shared_from_this<hubconnection>
{
public:
	hubconnection(boost::asio::io_service& io_service, hub& courier);
	bool init(const std::string& host, uint16_t port);
	bool write(const hubmessage& msg, bool wait = false);
	void close(bool forced);

private:

	void handle_read_header(const boost::system::error_code& error);
	void handle_read_body(const boost::system::error_code& error);
	void do_write(hubmessage msg);
	void handle_write(const boost::system::error_code& error);
	void do_close(bool forced);

private:
	boost::asio::io_service&	io_service_;
	tcp::socket					socket_;
	hub&						courier_;
	hubmessage					inmsg_;
	hubmessage_queue			outmsg_queue_;
	boost::atomic_bool			is_closing;
	boost::mutex				write_msgs_lock_;
};

#endif
