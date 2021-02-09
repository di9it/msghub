#include "hubconnection.h"


//#include "msgproc.h"
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
#include <iostream>

using boost::asio::ip::tcp;

hubconnection::hubconnection(boost::asio::io_service& io_service, hub& courier)
	: io_service_(io_service)
	, socket_(io_service)
	, courier_(courier)
    , is_closing(false)
{}

bool hubconnection::init(const std::string& host, uint16_t port)
{
	try
	{
		tcp::resolver resolver(io_service_);
		tcp::resolver::query query(host, boost::lexical_cast<std::string>(port));
		tcp::resolver::iterator iterator = resolver.resolve(query);

		// Do blocking connect (connection is more important than subscription here)
		boost::asio::connect(socket_, iterator);

		// Schedule packet read
		boost::asio::async_read(socket_,
			boost::asio::buffer(inmsg_.data(), inmsg_.header_length()),
			boost::bind(&hubconnection::handle_read_header, shared_from_this(),
			boost::asio::placeholders::error));
	}
	catch (std::exception&)
	{
		return false;
	}

	return true;
}

bool hubconnection::write(const hubmessage& msg, bool wait)
{
	try
	{
		if (wait)
		{
			boost::asio::write(socket_, boost::asio::buffer(msg.data(), msg.length()));
		}
		else
		{
			io_service_.post(boost::bind(&hubconnection::do_write, shared_from_this(), msg));
		}
        return true;
	}
	catch (std::exception&)
	{
		return false;
	}

}

void hubconnection::close(bool forced)
{
	io_service_.post(boost::bind(&hubconnection::do_close, shared_from_this(), forced));
}

void hubconnection::handle_read_header(const boost::system::error_code& error)
{
	if (!error && inmsg_.verify())
	{
		boost::asio::async_read(
			socket_,
			boost::asio::buffer(inmsg_.payload(), inmsg_.payload_length()),
			boost::bind(&hubconnection::handle_read_body, shared_from_this(),
			boost::asio::placeholders::error));
	}
	else
	{
		do_close(true);
	}
}

void hubconnection::handle_read_body(const boost::system::error_code& error)
{
	if (!error)
	{
		courier_.deliver(inmsg_);

		boost::asio::async_read(socket_,
			boost::asio::buffer(inmsg_.data(), inmsg_.header_length()),
			boost::bind(&hubconnection::handle_read_header, shared_from_this(),
			boost::asio::placeholders::error));
	}
	else
	{
		do_close(true);
	}
}

void hubconnection::do_write(hubmessage msg)
{
    bool iswriting = true;
    {
        boost::mutex::scoped_lock lock(write_msgs_lock_);
        iswriting = !outmsg_queue_.empty();
        outmsg_queue_.push_back(msg);
    }
	if (!iswriting)
	{
		boost::asio::async_write(socket_,
			boost::asio::buffer(outmsg_queue_.front().data(),
			outmsg_queue_.front().length()),
			boost::bind(&hubconnection::handle_write, shared_from_this(),
			boost::asio::placeholders::error));
	}
}

void hubconnection::handle_write(const boost::system::error_code& error)
{
	if (!error)
	{
        bool remaining_messages = false;
        {
            boost::mutex::scoped_lock lock(write_msgs_lock_);
            outmsg_queue_.pop_front();
            remaining_messages = !outmsg_queue_.empty();
        }
		if (remaining_messages)
		{
			boost::asio::async_write(socket_,
				boost::asio::buffer(outmsg_queue_.front().data(),
				outmsg_queue_.front().length()),
				boost::bind(&hubconnection::handle_write, shared_from_this(),
				boost::asio::placeholders::error));
        } else if (is_closing) {
            do_close(false);
        }
	}
	else
	{
		do_close(true);
	}
}

void hubconnection::do_close(bool forced)
{
    is_closing = true;

	// TODO: Unsubscribe?

    bool immediate = forced;
    if (!forced) {
        boost::mutex::scoped_lock lock(write_msgs_lock_);
        immediate |= outmsg_queue_.empty();
    }
    
    if (immediate) {
        if (socket_.is_open())
            socket_.close();
    }
}
