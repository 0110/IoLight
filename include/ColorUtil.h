/**
 * @brief ColorUtil collection
 * @file  ColorUtil.h
 * 
 */


#include <stdint.h>
#include <Adafruit_NeoPixel.h>

typedef enum dir_t {
    FORWARD = 0,
    BACKWARD
} Direction;


void RainbowCycle (Adafruit_NeoPixel* pix, uint8_t *pIndex);