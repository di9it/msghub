#include "hubclient.h"

//#include "msgproc.h"
//#include "msghub.h"
//#include "hubmessage.h"
//
//#include <memory>
//#include <cstdlib>
//#include <functional>
//#include <algorithm>
//#include <vector>
//#include <deque>
//
//#include <boost/bind.hpp>
//#include <boost/shared_ptr.hpp>
//#include <boost/enable_shared_from_this.hpp>
//#include <boost/asio.hpp>
//
//#include <boost/thread/thread.hpp>
//#include <boost/lexical_cast.hpp>
//#include <boost/thread/mutex.hpp>

using boost::asio::ip::tcp;

hubclient::hubclient(boost::asio::io_service& io_service, hub& distrib)
	: socket_(io_service)
	, distributor_(distrib)
{}

tcp::socket& hubclient::socket()
{
	return socket_;
}

void hubclient::start()
{
	// First read tiny header for verification
	boost::asio::async_read(socket_,
		boost::asio::buffer(inmsg_.data(), inmsg_.header_length()),
		boost::bind(
		&hubclient::handle_read_header, shared_from_this(),
		boost::asio::placeholders::error));
}

void hubclient::write(const hubmessage& msg)
{
	boost::mutex::scoped_lock lock(write_msgs_lock_);

	bool write_in_progress = !outmsg_queue_.empty();
	outmsg_queue_.push_back(msg);
	if (!write_in_progress)
	{
		boost::asio::async_write(socket_,
			boost::asio::buffer(outmsg_queue_.front().data(),
			outmsg_queue_.front().length()),
			boost::bind(&hubclient::handle_write, shared_from_this(),
			boost::asio::placeholders::error));
	}
}

void hubclient::handle_read_header(const boost::system::error_code& error)
{
	if (!error && inmsg_.verify())
	{
		// Decode header and schedule message handling itself
		boost::asio::async_read(socket_,
			boost::asio::buffer(inmsg_.payload(), inmsg_.payload_length()),
			boost::bind(&hubclient::handle_read_body, shared_from_this(),
			boost::asio::placeholders::error));
	}
	else
	{
		// error or wrong header - ignore the message
	}
}

void hubclient::handle_read_body(const boost::system::error_code& error)
{
	if (!error)
	{
		distributor_.distribute(shared_from_this(), inmsg_);

		// Get next
		boost::asio::async_read(socket_,
			boost::asio::buffer(inmsg_.data(), inmsg_.header_length()),
			boost::bind(&hubclient::handle_read_header, shared_from_this(),
			boost::asio::placeholders::error));
	}
	else
	{
		// error
	}
}

void hubclient::handle_write(const boost::system::error_code& error)
{
	if (!error)
	{
		boost::mutex::scoped_lock lock(write_msgs_lock_);
		outmsg_queue_.pop_front();
		if (!outmsg_queue_.empty())
		{
			// Write next from queue
			boost::asio::async_write(socket_,
				boost::asio::buffer(outmsg_queue_.front().data(),
				outmsg_queue_.front().length()),
				boost::bind(&hubclient::handle_write, shared_from_this(),
				boost::asio::placeholders::error));
		}
	}
	else
	{
		// error
	}
}
