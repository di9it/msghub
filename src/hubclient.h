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
	void write(const hubmessage& msg);
  private:
    using error_code = boost::system::error_code;
	void handle_read_header(error_code);
	void handle_read_body(error_code);
	void handle_write(error_code);

    auto bind(void (hubclient::*)(error_code));
  private:
	tcp::socket			socket_;
	ihub&				distributor_;
	hubmessage			inmsg_;
	hubmessage_queue	outmsg_queue_;
};

} }
