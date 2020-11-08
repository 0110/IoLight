/**
 * @file test_helper.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2020-11-08
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include <stdint.h>

#ifndef TEST_HELPER_FUNCTION
#define TEST_HELPER_FUNCTION
/* only usable when testing */
#ifdef UNIT_TEST

uint32_t replacementColorHSV(uint16_t hue, uint8_t sat, uint8_t val);

#endif
#endif /* TEST_HELPER_FUNCTION */