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

/**
 * @brief Test predefined text values
 * 
 */
void test_colorConversion(void) {
    TEST_ASSERT_EQUAL(0x00000000U, CALL_COLOREXTARCT("off"));
    TEST_ASSERT_EQUAL(0x00000000U, CALL_COLOREXTARCT("black"));
    TEST_ASSERT_EQUAL(0x00FF0000U, CALL_COLOREXTARCT("red"));
    TEST_ASSERT_EQUAL(0x0000FF00U, CALL_COLOREXTARCT("green"));
    TEST_ASSERT_EQUAL(0x000000FFU, CALL_COLOREXTARCT("blue"));
}

/**
 * @brief Test hex values, describing red, green blue
 * 
 */
void test_RRGGBB(void) {

                                                    /* RR GG BB */
    TEST_ASSERT_EQUAL(0x00000000U, CALL_COLOREXTARCT("#000000")); 
    TEST_ASSERT_EQUAL(0x00FF0000U, CALL_COLOREXTARCT("#FF0000"));
    TEST_ASSERT_EQUAL(0x0000FF00U, CALL_COLOREXTARCT("#00FF00"));
    TEST_ASSERT_EQUAL(0x000000FFU, CALL_COLOREXTARCT("#0000FF"));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_colorConversion);
    RUN_TEST(test_RRGGBB);
    UNITY_END();

    return 0;
}