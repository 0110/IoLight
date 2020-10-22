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
    TEST_ASSERT_EQUAL(0x00FFFFFFU, CALL_COLOREXTARCT("white"));
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


/**
 * @brief Test hex values, describing red, green blue
 * 
 */
void test_rrggbb(void) {

                                                    /* rr gg bb */
    TEST_ASSERT_EQUAL(0x00000000U, CALL_COLOREXTARCT("#000000")); 
    TEST_ASSERT_EQUAL(0x00FF0000U, CALL_COLOREXTARCT("#ff0000"));
    TEST_ASSERT_EQUAL(0x0000FF00U, CALL_COLOREXTARCT("#00ff00"));
    TEST_ASSERT_EQUAL(0x000000FFU, CALL_COLOREXTARCT("#0000ff"));
    TEST_ASSERT_EQUAL(0x00123456U, CALL_COLOREXTARCT("#123456"));
}

/**
 * @brief Test faulty values
 * 
 */
void test_noneColorValues(void) {
    TEST_ASSERT_EQUAL(0x00000000U, CALL_COLOREXTARCT("asdf"));
    TEST_ASSERT_EQUAL(0x00000000U, CALL_COLOREXTARCT("#asdasdas"));
    TEST_ASSERT_EQUAL(0x00000000U, CALL_COLOREXTARCT("#HelloWorld"));
    TEST_ASSERT_EQUAL(0x00000000U, CALL_COLOREXTARCT("#00AABBVV"));
    TEST_ASSERT_EQUAL(0x00000000U, CALL_COLOREXTARCT("#00AABBCCDD"));
    TEST_ASSERT_EQUAL(0x00000000U, CALL_COLOREXTARCT("#00ccbbaaff"));
    TEST_ASSERT_EQUAL(0x00000000U, extractColor(NULL, 7));
    TEST_ASSERT_EQUAL(0x00000000U, extractColor("red", 7));
    TEST_ASSERT_EQUAL(0x00000000U, extractColor("#00ff00", 6));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_colorConversion);
    RUN_TEST(test_RRGGBB);
    RUN_TEST(test_rrggbb);
    RUN_TEST(test_noneColorValues);
    UNITY_END();

    return 0;
}