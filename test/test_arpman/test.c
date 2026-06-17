#include "unity.h"
#include "fake_arpman.h"
#include <stdio.h>
#include <assert.h>

void setUp(void)
{}

void tearDown(void)
{}

void test_arpman_create_context(void)
{
	status_t _stat = SUCCESS;
	arpman_context_t context = {
		.ifindex = 1,
		.timeout = 1000,
		.src_ip = {127, 0, 0, 1},
		.src_mac = {204, 71, 64, 252, 123, 5},
		.out_mac = {0, 0, 0, 0, 0, 0}

	};
	_stat = arpman_create_context(&context);
	if (errno)
		printf("errno : %s\n", strerror(errno));
	TEST_ASSERT_EQUAL(SUCCESS, _stat);
	_stat = arpman_delete_context(&context);
	if (errno)
		printf("errno : %s\n", strerror(errno));
	TEST_ASSERT_EQUAL(SUCCESS, _stat);
	return;
}

void test_arpman_request_mac0(void)
{
	// All functions in this test supposed to work fine
	status_t _stat = SUCCESS;
	uint8_t new_mac[6];
	arpman_context_t context = {
		.sockfd = 3, // value 3 as fd makes the fake recvfrom() and fake poll() to work fine
		.ifindex = 0,
		.timeout = 1000,
		.src_ip = {127, 0, 0, 1},
		.src_mac = {255, 255, 255, 255, 255, 255},
		.out_mac = {0, 0, 0, 0, 0, 0}
	};
	_stat = arpman_request_mac(&context, (uint32_t) 0, new_mac); // the two last args don't matter
	TEST_ASSERT_EQUAL(SUCCESS, _stat);
	return;
}

void test_arpman_request_mac1(void)
{
	// In this test fake poll() must work fine but the fake recvfrom() must fail and I expect ERRRECV
	status_t _stat = SUCCESS;
	uint8_t new_mac[6];
	arpman_context_t context = {
		.sockfd = 2, // value 3 as fd makes the fake recvfrom() and fake poll() to work fine
		.ifindex = 0,
		.timeout = 1000,
		.src_ip = {127, 0, 0, 1},
		.src_mac = {255, 255, 255, 255, 255, 255},
		.out_mac = {0, 0, 0, 0, 0, 0}
	};
	_stat = arpman_request_mac(&context, (uint32_t) 0, new_mac); // the two last args don't matter
	TEST_ASSERT_EQUAL(ERRRECV, _stat);
	return;
}

void test_arpman_request_mac2(void)
{
	// In this test the fake recvfrom() must work fine and the fake poll() must timeout. I expect TIMEOUT
	status_t _stat = SUCCESS;
	uint8_t new_mac[6];
	arpman_context_t context = {
		.sockfd = 4, // value 3 as fd makes the fake recvfrom() and fake poll() to work fine
		.ifindex = 0,
		.timeout = 1000,
		.src_ip = {127, 0, 0, 1},
		.src_mac = {255, 255, 255, 255, 255, 255},
		.out_mac = {0, 0, 0, 0, 0, 0}
	};
	_stat = arpman_request_mac(&context, (uint32_t) 0, new_mac); // the two last args don't matter
	TEST_ASSERT_EQUAL(TIMEOUT, _stat);
	return;
}

void test_arpman_request_mac3(void)
{
	// In this test the fake recvfrom() must work fine and the fake poll() must fail. I expect ERRPOLL
	status_t _stat = SUCCESS;
	uint8_t new_mac[6];
	arpman_context_t context = {
		.sockfd = 5, // value 3 as fd makes the fake recvfrom() and fake poll() to work fine
		.ifindex = 0,
		.timeout = 1000,
		.src_ip = {127, 0, 0, 1},
		.src_mac = {255, 255, 255, 255, 255, 255},
		.out_mac = {0, 0, 0, 0, 0, 0}
	};
	_stat = arpman_request_mac(&context, (uint32_t) 0, new_mac); // the two last args don't matter
	TEST_ASSERT_EQUAL(ERRPOLL, _stat);
	return;
}


int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_arpman_create_context);
	RUN_TEST(test_arpman_request_mac0);
	RUN_TEST(test_arpman_request_mac1);
	RUN_TEST(test_arpman_request_mac2);
	RUN_TEST(test_arpman_request_mac3);
	return UNITY_END();
}
