#include "unity.h"
#include "utils/checksum.h"

void setUp(void)
{}

void tearDown(void)
{}

void test_case0(void)
{
	uint8_t d0[] = {0x12, 0x34, 0x56, 0x78};
	TEST_ASSERT_EQUAL(0x9753, checksum(d0, 4));
	return;
}

void test_case1(void)
{
	uint8_t d1[] = {0x12, 0x34, 0x56};
	TEST_ASSERT_EQUAL(0x97cb, checksum(d1, 3));
	return;
}

void test_case2(void)
{
	uint8_t d2[] = {0xff, 0xff, 0x01, 0x00};
	TEST_ASSERT_EQUAL(0xfeff, checksum(d2, 4));
	return;
}

void test_case3(void)
{
	uint8_t d3[] = {0x45, 0x00, 0x00, 0x54, 0x00, 0x00, 0x40, 0x00};
	TEST_ASSERT_EQUAL(0x7aab, checksum(d3, 8));
	return;
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_case0);
	RUN_TEST(test_case1);
	RUN_TEST(test_case2);
	RUN_TEST(test_case3);
	return UNITY_END();
}
