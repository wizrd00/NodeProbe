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
		.ip = {127, 0, 0, 1},
		.mac = {204, 71, 64, 252, 123, 5}
	};
	_stat = arpman_create_context(&context);
	TEST_ASSERT_EQUAL(SUCCESS, _stat);
	return;
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_arpman_create_context);
	return UNITY_END();
}
