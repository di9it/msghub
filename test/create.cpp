#include "msghub.h"

#include <algorithm>
#include <string>

#include <boost/asio.hpp>
#include <boost/test/test_tools.hpp>

void test_create()
{
	// Create 2 instances with same address and port, second shoul fail
	{
		boost::asio::io_service io_service;
		//boost::asio::io_service io_service2;
		
		msghub msghub1(io_service);
		BOOST_CHECK(msghub1.create(0xBEE));
		
		msghub msghub2(io_service);
		
		// Fail as port is in use by previous instance (-SO_REUSEPORT, issue on Windows)
		//BOOST_CHECK(!msghub2.create(0xBEE));

		BOOST_CHECK(msghub2.create(0xB0B));
		//io_service.run();
		//io_service2.run();
		io_service.stop();
		//io_service2.stop();
	}

}