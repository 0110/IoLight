/**
 * @brief ColorUtil collection
 * @file  ColorUtil.h
 * 
 */

#ifndef COLOR_UTIL
#define COLOR_UTIL

#include <stdint.h>
#include <Adafruit_NeoPixel.h>

typedef enum dir_t {
    FORWARD = 0,
    BACKWARD
} Direction;


void RainbowCycle (Adafruit_NeoPixel* pix, uint8_t *pIndex);

/**
 * @brief Extract color form a given string
 * possible values are:
 * - red
 * - green
 * - blue
 * - white
 * - black
 * - off
 * - RRGGBB     (red, green blue as hex values: 0-F (uppercase))
 * - rrggbb     (red, green blue as hex values: 0-f (lowercase))
 * @param text      The text with the color information
 * @param length    The amout of characters in the given text 
 * @return uint32_t 32-bit color value. Most significant byte is white (for RGBW
              pixels) or ignored (for RGB pixels), next is red, then green,
              and least significant byte is blue.
 */
uint32_t extractColor(const char *text, int length);

#endif /* COLOR_UTIL */