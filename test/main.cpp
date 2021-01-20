#include <boost/test/unit_test.hpp>

using namespace boost;
using namespace unit_test;

void test_create();
void test_connect();

void test_subscribe();
void test_unsubscribe();

void test_client_onserverfailure();
void test_server_oncleintfailure();

void test_toobigmsg();
void test_emptymsg();

test_suite* init_unit_test_suite(int, char**)
{
	test_suite *test = BOOST_TEST_SUITE("messagehub test");

	test->add(BOOST_TEST_CASE(&test_create));
	test->add(BOOST_TEST_CASE(&test_connect));
	test->add(BOOST_TEST_CASE(&test_subscribe));
	//test->add(BOOST_TEST_CASE(&test_unsubscribe));
	//test->add(BOOST_TEST_CASE(&test_client_onserverfailure));
	//test->add(BOOST_TEST_CASE(&test_server_oncleintfailure));
	//test->add(BOOST_TEST_CASE(&test_toobigmsg));
	//test->add(BOOST_TEST_CASE(&test_emptymsg));

	return test;
}
