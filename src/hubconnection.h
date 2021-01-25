#pragma once

#include "ihub.h"
#include "hubmessage.h"

#include <string>
#include <memory>
#include <functional>

#include <boost/atomic.hpp>
#include <boost/asio.hpp>

namespace msghublib { namespace detail {
using boost::asio::ip::tcp;

class hubconnection : public std::enable_shared_from_this<hubconnection>
{
public:
    template <typename Executor>
	hubconnection(Executor executor, ihub& courier)
        : socket_(make_strand(executor))
        , courier_(courier)
        , is_closing(false)
    {}

	bool init(const std::string& host, uint16_t port);
	bool write(const hubmessage& msg, bool wait = false);
	void close(bool forced);

private:
    using error_code = boost::system::error_code;
    auto bind(void (hubconnection::* /*handler*/)(error_code));

	void handle_read_header(error_code error);
	void handle_read_body(error_code error);
	void do_write(hubmessage msg);
	void handle_write(error_code error);
	void do_close(bool forced);

	tcp::socket        socket_;
	ihub&              courier_;
	hubmessage         inmsg_;
	hubmessage_queue   outmsg_queue_;
	boost::atomic_bool is_closing;
};

}  // namespace detail
}  // namespace msghublib
