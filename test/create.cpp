#include "msghub.h"

#include <algorithm>
#include <string>

#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(message_hub)

BOOST_AUTO_TEST_CASE(test_create)
{
    boost::asio::io_context io;

    msghublib::msghub msghub1(io.get_executor());
    BOOST_CHECK(msghub1.create(0xBEE));

    msghublib::msghub msghub2(io.get_executor());

    // Fail as port is in use by previous instance (-SO_REUSEPORT, issue on Windows)
    //BOOST_CHECK(!msghub2.create(0xBEE));

    BOOST_CHECK(msghub2.create(0xB0B));
    //io.run();
    //io_service2.run();
    io.stop();
    //io_service2.stop();
}

BOOST_AUTO_TEST_SUITE_END()
