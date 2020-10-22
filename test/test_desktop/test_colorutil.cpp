#include <unity.h>
#include <stdio.h>
#include <string.h>
#include "ColorUtil.h"


#define CALL_COLOREXTARCT(text)    extractColor(text, strlen(text))

void setUp(void)
{
  
}

void tearDown(void)
{
}

void test_colorConversion(void) {
    TEST_ASSERT_EQUAL(0, CALL_COLOREXTARCT("off"));
    TEST_ASSERT_EQUAL(0, CALL_COLOREXTARCT("black"));
    TEST_ASSERT_EQUAL(0, CALL_COLOREXTARCT("#000000")); /* RR GG BB */
    TEST_ASSERT_EQUAL(0x0F00U, CALL_COLOREXTARCT("red"));
    TEST_ASSERT_EQUAL(0x00F0U, CALL_COLOREXTARCT("green"));
    TEST_ASSERT_EQUAL(0x000FU, CALL_COLOREXTARCT("blue"));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_colorConversion);
    UNITY_END();

    return 0;
}