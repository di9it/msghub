#include "msghub.h"

#include <mutex>
#include <condition_variable> 

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/test/unit_test_monitor.hpp>
#include <boost/test/test_tools.hpp>

std::mutex mutant;
std::condition_variable newmessage;
bool goodmessage = false;

using namespace boost::unit_test;
void test_create_on_message(const std::string& topic, std::vector<char>& message)
{
	std::unique_lock<std::mutex> lock(mutant);
	std::string expected("$testmessage$");
	goodmessage = std::equal(expected.begin(), expected.end(), message.begin());
	BOOST_CHECK(goodmessage);
	newmessage.notify_one();
}

bool publish_message(msghub& msghub)
{
	BOOST_CHECK(msghub.publish("test_topic", "$testmessage$"));
	std::unique_lock<std::mutex> lock(mutant);
	newmessage.wait(lock);
	BOOST_CHECK(goodmessage);
	return true;
}

void test_subscribe()
{
	boost::asio::io_service io_service;
	msghub msghub(io_service);
	BOOST_CHECK(msghub.create(0xBEE));

	BOOST_CHECK(msghub.subscribe("test_topic", test_create_on_message));

	unit_test_monitor_t& monitor = unit_test_monitor_t::instance();
	monitor.p_timeout.set(1);
	monitor.execute(boost::bind(publish_message, msghub));
	io_service.stop();
	//io_service1.stop();
}