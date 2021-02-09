#ifndef _MSGHUB_MSGHUB_IMPL_H_
#define _MSGHUB_MSGHUB_IMPL_H_

#include "hub.h"
#include "msghub.h"
#include "hubconnection.h"
#include "hubclient.h"
#include "hubmessage.h"

#include <memory>
#include <cstdlib>
#include <functional>
#include <algorithm>
#include <vector>
#include <map>

using boost::asio::ip::tcp;

class msghub_impl
  : public hub,
    public std::enable_shared_from_this<msghub_impl>
{
  public:
    using any_io_executor = boost::asio::any_io_executor;

  private:
	std::map<std::string, msghub::onmessage> messagemap_;

  private:
	tcp::acceptor acceptor_;
    boost::asio::executor_work_guard<any_io_executor> work_;
	std::shared_ptr<hubconnection> publisher_;
	bool initok_;

  public:
	msghub_impl(any_io_executor io_service);
	~msghub_impl();
	bool connect(const std::string& hostip, uint16_t port);
	bool create(uint16_t port);
	bool publish(std::string_view topic, const_charbuf message);

	bool unsubscribe(const std::string& topic);
	bool subscribe(const std::string& topic, msghub::onmessage handler);

    void stop();

  public:
	std::multimap<std::string, std::shared_ptr<hubclient> > subscribers_;

	void distribute(std::shared_ptr<hubclient> const& subscriber, hubmessage const& msg);
	void deliver(hubmessage const& msg);
	void accept_next();
	void handle_accept(std::shared_ptr<hubclient> const& session, const boost::system::error_code& error);
};

#endif
