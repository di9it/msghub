#include "msghub.h"
#include "msghub_impl.h"
//#include 

//#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/asio.hpp>


msghub::msghub(boost::asio::io_service& s)
{
	pimpl = boost::make_shared<msghub_impl>(s);
}

bool msghub::connect(const std::string& hostip, uint16_t port, uint8_t threads)
{
	return pimpl->connect(hostip, port, threads);
}

bool msghub::create(uint16_t port, uint8_t threads)
{
	return pimpl->create(port, threads);
}

bool msghub::unsubscribe(const std::string& topic)
{
	return pimpl->unsubscribe(topic);
}

bool msghub::subscribe(const std::string& topic, onmessage handler)
{
	return pimpl->subscribe(topic, handler);
}

bool msghub::publish(const std::string& topic, const std::vector<char>& message)
{
	return pimpl->publish(topic, message);
}

bool msghub::publish(const std::string& topic, const std::string& message)
{
	return pimpl->publish(topic, message);
}

void msghub::join()
{
	return pimpl->join();
}
