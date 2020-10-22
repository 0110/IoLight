/**
 * @brief ColorUtil collection
 * @file  ColorUtil.c
 * 
 * See:
 * https://learn.adafruit.com/multi-tasking-the-arduino-part-3/utility-functions
 */
#include "ColorUtil.h"
#include "string.h"
#include <stdio.h>

#define MIN(a, b)   ((a) > (b) ? b : a)
#define COMPARE_STR(text,length, staticText)    strncmp(text, staticText, MIN(length, (int) strlen(staticText)))

#ifndef UNIT_TEST
#include <Adafruit_NeoPixel.h>

/*!
    @brief   Convert separate red, green and blue values into a single
             "packed" 32-bit RGB color.
    @param   r  Red brightness, 0 to 255.
    @param   g  Green brightness, 0 to 255.
    @param   b  Blue brightness, 0 to 255.

    Copied from Adafruit_NeoPixel.h

    @return  32-bit packed RGB value, which can then be assigned to a
             variable for later use or passed to the setPixelColor()
             function. Packed RGB format is predictable, regardless of
             LED strand color order.
  */
uint32_t   Color(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

/**
 * @brief 
 * 
 * @param WheelPos 
 * 
 * The colours are a transition r - g - b - back to r
 * 
 * @return uint32_t 
 */
uint32_t Wheel(uint8_t WheelPos)
{
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85)
    {
        return Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    else if(WheelPos < 170)
    {
        WheelPos -= 85;
        return Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    else
    {
        WheelPos -= 170;
        return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
}

void RainbowCycle (Adafruit_NeoPixel* pix, uint8_t *pIndex)
{
    uint8_t Index = (*pIndex);
    for(int i=0; i< pix->numPixels(); i++)
    {
        pix->setPixelColor(i, Wheel(((i * 256 / pix->numPixels()) + Index) & 255));
    }
    (*pIndex) = Index + 1;
    pix->show();
}

#endif


uint32_t extractColor(const char *text, int length)  {
    /* invalid values are returned as black */
    if ((length <= 0) ||
        (text == NULL) ||
        (strlen(text) < length)){
        return 0;
    }

    if (COMPARE_STR(text, length, "red") == 0) {
        return 0x00FF0000;
    } else if (COMPARE_STR(text, length, "green") == 0) {
        return 0x0000FF00;
    } else if (COMPARE_STR(text, length, "blue") == 0) {
        return 0x000000FF;
    } else if (COMPARE_STR(text, length, "white") == 0) {
        return 0x00FFFFFF;
    }  else if (text[0] == '#' && length == 7) { /* parse #rrggbb or #RRGGBB */
        int red, green, blue = 0;
        int parsed = sscanf(text, "#%2X%2X%2X", &red, &green, &blue);
        if (parsed == 3) {
            uint32_t c = blue;
            c |= (green << 8);
            c |= (red << 16);
#ifdef UNIT_TEST
    printf("rrggbb %s = %x\n", text, c);
#endif
            return c;
        } else {
            /* try to parse lower case hex values */
            parsed = sscanf(text, "#%2x%2x%2x", &red, &green, &blue);
            if (parsed == 3) {
                uint32_t c = blue;
                c |= (green << 8);
                c |= (red << 16);
#ifdef UNIT_TEST
    printf("RRGGBB %s = %x\n", text, c);
#endif
                return c;
            } else {
                return 0;
            }
        }
    }

    return 0;
}