#include "hubclient.h"

using boost::asio::ip::tcp;

auto hubclient::bind(void (hubclient::*handler)(error_code)) {
#pragma GCC diagnostic ignored "-Wdeprecated" // implicit this-capture
    return [=, self = shared_from_this()](error_code ec, size_t /*transferred*/) {
        (this->*handler)(ec);
    };
}

tcp::socket& hubclient::socket() {
	return socket_;
}

void hubclient::start()
{
	// First read tiny header for verification
	async_read(socket_,
		boost::asio::buffer(inmsg_.data(), inmsg_.header_length()),
        bind(&hubclient::handle_read_header));
}

void hubclient::write(const hubmessage& msg)
{
    post(socket_.get_executor(), [this, msg, self=shared_from_this()]{
        bool write_in_progress = !outmsg_queue_.empty();
        outmsg_queue_.push_back(msg);
        if (!write_in_progress)
        {
            async_write(socket_,
                boost::asio::buffer(outmsg_queue_.front().data(),
                outmsg_queue_.front().length()),
                bind(&hubclient::handle_write));
        }
    });
}

void hubclient::handle_read_header(error_code error)
{
	if (!error && inmsg_.verify())
	{
		// Decode header and schedule message handling itself
		boost::asio::async_read(socket_,
			boost::asio::buffer(inmsg_.payload()),
			bind(&hubclient::handle_read_body));
	}
	else
	{
		// error or wrong header - ignore the message
	}
}

void hubclient::handle_read_body(error_code error)
{
	if (!error)
	{
		distributor_.distribute(shared_from_this(), inmsg_);

		// Get next
		boost::asio::async_read(socket_,
			boost::asio::buffer(inmsg_.data(), inmsg_.header_length()),
			bind(&hubclient::handle_read_header));
	}
	else
	{
		// error
	}
}

void hubclient::handle_write(error_code error)
{
	if (!error) {
		outmsg_queue_.pop_front();
		if (!outmsg_queue_.empty())
		{
			// Write next from queue // TODO remove duplication
            async_write(socket_,
                boost::asio::buffer(outmsg_queue_.front().data(),
                outmsg_queue_.front().length()),
                bind(&hubclient::handle_write));
		}
	}
	else
	{
		// error TODO handling
	}
}
