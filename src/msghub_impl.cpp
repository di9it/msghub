#include "msghub_impl.h"

#include "hub.h"
#include "msghub_impl.h"
#include "hubconnection.h"
#include "hubclient.h"
#include "hubmessage.h"

#include <memory>
#include <cstdlib>
#include <functional>
#include <algorithm>
#include <vector>

#include <vector>
#include <map>
#include <set>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>

#include <boost/thread/thread.hpp>
#include <boost/lexical_cast.hpp>

using boost::asio::ip::tcp;


void msghub_impl::initpool(uint8_t threads)
{
	if (!threads)
		threads++;

	// Create a pool of threads to run all of the io_services.
	while (threads--)
	{
		boost::shared_ptr<boost::thread> thread(
			new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_)));

		threads_.push_back(thread);
	}
}

msghub_impl::msghub_impl(boost::asio::io_service& io_service)
	: publisher_(new hubconnection(io_service, *this))
	, io_service_(io_service)
    , work_(boost::asio::io_service::work(io_service))
	, acceptor_(io_service)
	, initok_(false)
{}

msghub_impl::~msghub_impl()
{
    join();
}


bool msghub_impl::connect(const std::string& hostip, uint16_t port, uint8_t threads)
{
	initpool(threads);
	initok_ = publisher_->init(hostip, port);
	return initok_;
}

bool msghub_impl::create(uint32_t port, uint8_t threads)
{
	tcp::endpoint endpoint(tcp::v4(), port);
	acceptor_ = tcp::acceptor(io_service_, endpoint);

    acceptor_.set_option(tcp::acceptor::reuse_address(true));
	acceptor_.listen();

	accept_next();

	initpool(threads);
	initok_ = publisher_->init("localhost", port);
	return initok_;
}

void msghub_impl::join()
{
    if (publisher_)
        publisher_->close(false);
    publisher_.reset();
    if (acceptor_.is_open()) 
        acceptor_.cancel();
    work_.reset();

	// Join to all service threads
	for (auto it : threads_)
        if (it->joinable())
            it->join();
}

bool msghub_impl::publish(const std::string& topic, const std::vector<char>& message)
{
	if (!initok_)
		return false;

	hubmessage msg;
	msg.set_message(topic, message);
	msg.set_action(hubmessage::action::publish);
	publisher_->write(msg);

	return true;
}

bool msghub_impl::publish(const std::string& topic, const std::string& message)
{
	std::vector<char> data;
	std::copy(message.begin(), message.end(), back_inserter(data));
	return publish(topic, data);
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

	messagemapit it = messagemap_.find(topic);
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

void msghub_impl::distribute(boost::shared_ptr<hubclient> subscriber, hubmessage& msg)
{
	boost::mutex::scoped_lock lock(subscriberslock_);

	std::string topic(msg.payload(), msg.topic_length());
	subscribersit it = subscribers_.find(topic);
	switch (msg.get_action())
	{
	case hubmessage::action::publish:
		if (it != subscribers_.end())
		{
			for (auto s : it->second)
				s->write(msg);
		}
		break;

	case hubmessage::action::subscribe:
		if (it != subscribers_.end())
		{
			it->second.insert(subscriber);
		}
		else
		{
			subscriberset newsubscribers;
			newsubscribers.insert(subscriber);
			subscribers_.insert(std::make_pair(topic, newsubscribers));
		}
		break;

	case hubmessage::action::unsubscribe:
		if (it != subscribers_.end())
		{
			it->second.erase(subscriber);
			if (!it->second.size())
			{
				subscribers_.erase(it);
			}
		}
		break;

	default:
		break;
	}
}

void msghub_impl::deliver(hubmessage& msg)
{
	std::string topic(msg.payload(), msg.topic_length());
	messagemapit it = messagemap_.find(topic);
	if (it != messagemap_.end())
	{
		std::vector<char> message(msg.body(), msg.body() + msg.body_length());
		it->second(topic, message);
	}
}

void msghub_impl::accept_next()
{
	boost::shared_ptr<hubclient> subscriber(new hubclient(io_service_.get_executor(), *this));

	// Schedule next accept
	acceptor_.async_accept(subscriber->socket(),
		boost::bind(&msghub_impl::handle_accept, this, subscriber,
		boost::asio::placeholders::error));
}

void msghub_impl::handle_accept(boost::shared_ptr<hubclient> client, const boost::system::error_code& error)
{
	if (!error)
	{
		client->start();
	}
	else
	{
		//// TODO: Handle IO error - on thread exit
		//int e = error.value();
	}

	accept_next();
}
