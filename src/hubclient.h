#pragma once

#include "ihub.h"
#include "hubmessage.h"

#include <memory>
#include <functional>
#include <deque>

#include <boost/asio.hpp>

namespace msghublib { namespace detail {
using boost::asio::ip::tcp;

class hubclient : public std::enable_shared_from_this<hubclient>
{
  public:
    template <typename Executor>
	hubclient(Executor executor, ihub& distrib)
      : socket_(make_strand(executor))
      , distributor_(distrib)
    {}

	tcp::socket& socket();
	void start();
	void stop();
	void write(const hubmessage& msg);
  private:
    using error_code = boost::system::error_code;
	void handle_read_header(error_code /*error*/);
	void handle_read_body(error_code /*error*/);
	void handle_write(error_code /*error*/);

    auto bind(void (hubclient::* /*handler*/)(error_code));
  
	tcp::socket			socket_;
	ihub&				distributor_;
	hubmessage			inmsg_;
	hubmessage_queue	outmsg_queue_;
};

}  // namespace detail
}  // namespace msghublib
