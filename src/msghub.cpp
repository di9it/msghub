#include "msghub.h"
#include "msghub_impl.h"
//#include 

#include <boost/asio.hpp>

namespace msghublib {

    msghub::~msghub() = default;

    /*explicit*/ msghub::msghub(boost::asio::any_io_executor executor)
     : pimpl(std::make_shared<detail::msghub_impl>(executor))
    { }

    void msghub::stop() {
        return pimpl->stop();
    }

    bool msghub::connect(const std::string& hostip, uint16_t port)
    {
        return pimpl->connect(hostip, port);
    }

    bool msghub::create(uint16_t port)
    {
        return pimpl->create(port);
    }

    bool msghub::unsubscribe(const std::string& topic)
    {
        return pimpl->unsubscribe(topic);
    }

    bool msghub::subscribe(const std::string& topic, onmessage handler)
    {
        return pimpl->subscribe(topic, handler);
    }

    bool msghub::publish(std::string_view topic, span<char const> message)
    {
        return pimpl->publish(topic, message);
    }

}
