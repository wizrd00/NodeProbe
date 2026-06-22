#include "unity.h"
#include "tcpman.h"
#include <stdio.h>

void setUp(void)
{}

void tearDown(void)
{}

void test_tcpman_create_context(void)
{
	status_t _stat = SUCCESS;
	tcpman_context_t context = {
		.ifindex = 1
	};
	_stat = tcpman_create_context(&context);
	if (errno)
		printf("errno : %s\n", strerror(errno));
	TEST_ASSERT_EQUAL(SUCCESS, _stat);
	_stat = tcpman_delete_context(&context);
	if (errno)
		printf("errno : %s\n", strerror(errno));
	TEST_ASSERT_EQUAL(SUCCESS, _stat);
	return;
}

void test_tcpman_sync_request0(void)
{
	// All functions in this test supposed to work fine
	status_t _stat = SUCCESS;
	tcpman_context_t context = {
		.sockfd = 3, // value 3 as fd makes the fake recvfrom() and fake poll() to work fine
		.ifindex = 0,
		.timeout = 1000,
		.mtu_size = 1500,
		.src_ip = {127, 0, 0, 1},
		.dst_ip = {127, 0, 0, 1},
		.src_mac = {255, 255, 255, 255, 255, 255},
		.dst_mac = {0, 0, 0, 0, 0, 0},
		.src_port = 0x1337,
		.dst_port = 22
	};
	_stat = tcpman_sync_request(&context); // the two last args don't matter
	TEST_ASSERT_EQUAL(SUCCESS, _stat);
	return;
}

void test_tcpman_sync_request1(void)
{
	// In this test fake poll() must work fine but the fake recvfrom() must fail and I expect ERRRECV
	status_t _stat = SUCCESS;
	tcpman_context_t context = {
		.sockfd = 2, // value 2 as fd makes the fake recvfrom() fail and fake poll() to work fine
		.ifindex = 0,
		.timeout = 1000,
		.mtu_size = 1500,
		.src_ip = {127, 0, 0, 1},
		.dst_ip = {127, 0, 0, 1},
		.src_mac = {255, 255, 255, 255, 255, 255},
		.dst_mac = {0, 0, 0, 0, 0, 0},
		.src_port = 0x1337,
		.dst_port = 22
	};
	_stat = tcpman_sync_request(&context); // the two last args don't matter
	TEST_ASSERT_EQUAL(ERRRECV, _stat);
	return;
}

void test_tcpman_sync_request2(void)
{
	// In this test the fake recvfrom() must work fine and the fake poll() must timeout. I expect TIMEOUT
	status_t _stat = SUCCESS;
	tcpman_context_t context = {
		.sockfd = 4, // value 4 as fd makes the fake recvfrom() work fine and fake poll() fail
		.ifindex = 0,
		.timeout = 1000,
		.mtu_size = 1500,
		.src_ip = {127, 0, 0, 1},
		.dst_ip = {127, 0, 0, 1},
		.src_mac = {255, 255, 255, 255, 255, 255},
		.dst_mac = {0, 0, 0, 0, 0, 0},
		.src_port = 0x1337,
		.dst_port = 22
	};
	_stat = tcpman_sync_request(&context); // the two last args don't matter
	TEST_ASSERT_EQUAL(TIMEOUT, _stat);
	return;
}

void test_tcpman_sync_request3(void)
{
	// In this test the fake recvfrom() must work fine and the fake poll() must fail. I expect ERRPOLL
	status_t _stat = SUCCESS;
	tcpman_context_t context = {
		.sockfd = 5, // value 5 as fd makes the fake recvfrom() work find and fake poll() fail
		.ifindex = 0,
		.timeout = 1000,
		.mtu_size = 1500,
		.src_ip = {127, 0, 0, 1},
		.dst_ip = {127, 0, 0, 1},
		.src_mac = {255, 255, 255, 255, 255, 255},
		.dst_mac = {0, 0, 0, 0, 0, 0},
		.src_port = 0x1337,
		.dst_port = 22
	};
	_stat = tcpman_sync_request(&context); // the two last args don't matter
	TEST_ASSERT_EQUAL(ERRPOLL, _stat);
	return;
}

void test_tcpman_sync_request4(void)
{
	// In this test the received segment must TCP with RST flag
	status_t _stat = SUCCESS;
	tcpman_context_t context = {
		.sockfd = 0, // value 0 as fd makes the fake recvfrom() and fake poll() to work fine
		.ifindex = 0,
		.timeout = 1000,
		.mtu_size = 1500,
		.src_ip = {127, 0, 0, 1},
		.dst_ip = {127, 0, 0, 1},
		.src_mac = {255, 255, 255, 255, 255, 255},
		.dst_mac = {0, 0, 0, 0, 0, 0},
		.src_port = 0x1337,
		.dst_port = 22
	};
	_stat = tcpman_sync_request(&context); // the two last args don't matter
	TEST_ASSERT_EQUAL(FAILURE, _stat);
	return;
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_tcpman_create_context);
	RUN_TEST(test_tcpman_sync_request0);
	RUN_TEST(test_tcpman_sync_request1);
	RUN_TEST(test_tcpman_sync_request2);
	RUN_TEST(test_tcpman_sync_request3);
	RUN_TEST(test_tcpman_sync_request4);
	return UNITY_END();
}
