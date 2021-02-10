#include "msghub.h"

#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(message_hub)

BOOST_AUTO_TEST_CASE(test_create)
{
    boost::asio::io_context io;

    msghublib::msghub msghub1(io.get_executor());
    BOOST_CHECK_NO_THROW(msghub1.create(0xBEE));

    // Fail as port is in use by previous instance (-SO_REUSEPORT, issue on Windows)
    using SE = msghublib::system_error;
    auto holds = [](auto code) {
        return [=](SE const& se) {
            return se.code() == code;
        };
    };

    {
        msghublib::msghub msghub2(io.get_executor());
        BOOST_CHECK_EXCEPTION(msghub2.create(0xBEE), SE, holds(boost::asio::error::address_in_use));
        BOOST_CHECK_EXCEPTION(msghub2.create(0xB0B), SE, holds(boost::asio::error::already_open));
    }

    {
        msghublib::msghub msghub3(io.get_executor());
        BOOST_CHECK_NO_THROW(msghub3.create(0xB0B));
    }
    //io.run();
    //io_service2.run();
    io.stop();
    //io_service2.stop();
}

BOOST_AUTO_TEST_SUITE_END()
