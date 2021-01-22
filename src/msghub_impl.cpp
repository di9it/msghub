#include "msghub_impl.h"

#include "hub.h"
#include "msghub_impl.h"
#include "hubconnection.h"
#include "hubclient.h"
#include "hubmessage.h"

#include <charbuf.h>
#include <memory>
#include <cstdlib>
#include <functional>
#include <algorithm>
#include <vector>

#include <vector>
#include <map>

#include <boost/asio.hpp>
#include <boost/range/iterator_range.hpp>

using boost::asio::ip::tcp;

msghub_impl::msghub_impl(boost::asio::any_io_executor executor)
	: acceptor_(make_strand(executor))
    , work_(make_work_guard(executor))
    , publisher_(std::make_shared<hubconnection>(executor, *this))
	, initok_(false)
{}

void msghub_impl::stop()
{
    if (publisher_)
        publisher_->close(false);

    publisher_.reset();

    if (acceptor_.is_open()) 
        acceptor_.cancel();

    work_.reset();
}

msghub_impl::~msghub_impl()
{
    stop();
}


bool msghub_impl::connect(const std::string& hostip, uint16_t port)
{
	initok_ = publisher_->init(hostip, port);
	return initok_;
}

bool msghub_impl::create(uint16_t port)
{
    acceptor_.open(tcp::v4());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
	acceptor_.bind({{}, port});
	acceptor_.listen();

	accept_next();

	initok_ = publisher_->init("localhost", port);
	return initok_;
}

bool msghub_impl::publish(std::string_view topic, const_charbuf message)
{
	if (!initok_)
		return false;

	hubmessage msg;
	msg.set_message(topic, message);
	msg.set_action(hubmessage::action::publish);
	publisher_->write(msg);

	return true;
}

bool msghub_impl::unsubscribe(const std::string& topic)
{
	if (!initok_)
		return false;

	if (messagemap_.find(topic) != messagemap_.end())
	{
		hubmessage msg;
		msg.set_message(topic);
		msg.set_action(hubmessage::action::unsubscribe);
		publisher_->write(msg, true);
	}
	return true;
}

bool msghub_impl::subscribe(const std::string& topic, msghub::onmessage handler)
{
	if (!initok_)
		return false;

	auto it = messagemap_.find(topic);
	if (it != messagemap_.end())
	{
		// Overwrite
		it->second = handler;
	}
	else
	{
		// New handler
		messagemap_.insert(std::make_pair(topic, handler));
	}

	hubmessage msg;
	msg.set_message(topic);
	msg.set_action(hubmessage::action::subscribe);
	publisher_->write(msg, true);

	// TODO: wait feedback form server here?
	return true;
}

void msghub_impl::distribute(std::shared_ptr<hubclient> const& subscriber, hubmessage const& msg)
{
    //post(acceptor_.get_executor(), do
	std::string topic(msg.topic());
	auto range = boost::make_iterator_range(subscribers_.equal_range(topic));

	switch (msg.get_action())
	{
	case hubmessage::action::publish:
        for (auto& [t,s] : range)
            s->write(msg);
        break;

	case hubmessage::action::subscribe:
        subscribers_.emplace(topic, subscriber);
		break;

	case hubmessage::action::unsubscribe:
        for (auto it = range.begin(); it != range.end();) {
            if (it->second == subscriber)
                it = subscribers_.erase(it);
            else
                ++it;
		}
		break;

	default:
		break;
	}
}

void msghub_impl::deliver(hubmessage const& msg)
{ 
	if (auto it = messagemap_.find(std::string(msg.topic()));
            it != messagemap_.end())
    {
		it->second(msg.topic(), msg.body());
	}
}

void msghub_impl::accept_next()
{
    auto subscriber = std::make_shared<hubclient>(acceptor_.get_executor(), *this);

	// Schedule next accept
	acceptor_.async_accept(subscriber->socket(),
        [=, this, self = shared_from_this()](boost::system::error_code ec) {
            handle_accept(subscriber, ec);
        });
}

void msghub_impl::handle_accept(std::shared_ptr<hubclient> const& client, const boost::system::error_code& error)
{
	if (!error)
	{
		client->start();
        accept_next();
	}
	else
	{
		//// TODO: Handle IO error - on thread exit
		//int e = error.value();
	}
}
