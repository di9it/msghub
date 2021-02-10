#include "msghub.h"

#include <algorithm>
#include <string>

#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>
namespace mh = msghublib;

BOOST_AUTO_TEST_SUITE(message_hub)
BOOST_AUTO_TEST_CASE(test_connect) {
    boost::asio::io_context io;

    mh::msghub hub(io.get_executor());

    BOOST_CHECK_NO_THROW(hub.create(0xBEE));
    BOOST_CHECK_NO_THROW(hub.connect("localhost", 0xBEE));
    io.stop();
}

BOOST_AUTO_TEST_SUITE_END()
