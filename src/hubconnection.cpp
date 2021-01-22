#include "hubconnection.h"

using boost::asio::ip::tcp;

auto hubconnection::bind(void (hubconnection::*handler)(error_code)) {
#pragma GCC diagnostic ignored "-Wdeprecated" // implicit this-capture
    return [=, self=shared_from_this()](error_code ec, size_t /*transferred*/) {
        (this->*handler)(ec);
    };
}

bool hubconnection::init(const std::string& host, uint16_t port)
{
	try
	{
		tcp::resolver resolver(socket_.get_executor());
        tcp::resolver::results_type results = resolver.resolve(host, std::to_string(port));

		// Do blocking connect (connection is more important than subscription here)
		connect(socket_, results);

		// Schedule packet read
		async_read(socket_,
			boost::asio::buffer(&inmsg_.headers(), inmsg_.header_length()),
			bind(&hubconnection::handle_read_header));
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
#pragma GCC diagnostic ignored "-Wdeprecated" // implicit this-capture
			post(socket_.get_executor(), [=, self=shared_from_this()]
                { do_write(msg); });
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
#pragma GCC diagnostic ignored "-Wdeprecated" // implicit this-capture
	post(socket_.get_executor(), [=, self=shared_from_this()]
        { do_close(forced); });
}

void hubconnection::handle_read_header(error_code error)
{
	if (!error && inmsg_.verify())
	{
		async_read(
			socket_,
			boost::asio::buffer(inmsg_.payload()),
			bind(&hubconnection::handle_read_body));
	}
	else
	{
		do_close(true);
	}
}

void hubconnection::handle_read_body(error_code error)
{
	if (!error)
	{
		courier_.deliver(inmsg_);

		async_read(socket_,
			boost::asio::buffer(inmsg_.data(), inmsg_.header_length()),
			bind(&hubconnection::handle_read_header));
	}
	else
	{
		do_close(true);
	}
}

void hubconnection::do_write(hubmessage msg)
{
	if (outmsg_queue_.push_back(msg); 1 == outmsg_queue_.size())
	{
		async_write(socket_,
			boost::asio::buffer(outmsg_queue_.front().data(),
			outmsg_queue_.front().length()),
			bind(&hubconnection::handle_write));
	}
}

void hubconnection::handle_write(error_code error)
{
	if (!error)
	{
        if (outmsg_queue_.pop_front(); !outmsg_queue_.empty())
		{
			async_write(socket_,
				boost::asio::buffer(outmsg_queue_.front().data(),
				outmsg_queue_.front().length()),
				bind(&hubconnection::handle_write));
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
    is_closing = true; // atomic

	// TODO: Unsubscribe?

    if (forced || outmsg_queue_.empty()) {
        if (socket_.is_open()) {
            boost::system::error_code ec;
            socket_.close(ec);
        }
    }
}
