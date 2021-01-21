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
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include <boost/lexical_cast.hpp>

using boost::asio::ip::tcp;

class hubconnection : public boost::enable_shared_from_this<hubconnection>
{
public:
    template <typename Executor>
	hubconnection(Executor executor, hub& courier)
        : socket_(make_strand(executor))
        , courier_(courier)
        , is_closing(false)
    {}

	bool init(const std::string& host, uint16_t port);
	bool write(const hubmessage& msg, bool wait = false);
	void close(bool forced);

private:
    using error_code = boost::system::error_code;
    auto bind(void (hubconnection::*)(error_code));

	void handle_read_header(error_code error);
	void handle_read_body(error_code error);
	void do_write(hubmessage msg);
	void handle_write(error_code error);
	void do_close(bool forced);

private:
	tcp::socket					socket_;
	hub&						courier_;
	hubmessage					inmsg_;
	hubmessage_queue			outmsg_queue_;
	boost::atomic_bool			is_closing;
};

#endif
