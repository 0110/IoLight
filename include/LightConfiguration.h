/**
 * @brief Light configuration
 * @file  LightConfiguration.h
 * 
 */

#ifndef LIGHT_CONFIGURATION_H
#define LIGHT_CONFIGURATION_H


#define FIRMWARE_VERSION "0.3.1"

#define NUMBER_LEDS 6

#define BLINK_INTERVAL  500 /**< Milliseconds */
#define RESET_TRIGGER   2048
#define PWM_MAXVALUE    1023
#define FADE_MAXVALUE   255

#define TIME_UNDEFINED  0xFFFFFFFFU

#define GPIO_BUTTON     D0  /**< Input button */
#define GPIO_WS2812     D1  /**< RGB LEDs */
#define GPIO_PIR        D6  /**< Passive infrared sensor */
#define GPIO_LED        D2  /**< None RGB light, controlled by mosfet */

HomieSetting<long> ledAmount("leds", "Amount of LEDs (of type WS2812); Range 1 to 2047");
HomieSetting<bool> motionActivation("motion", "Activate light (none RGB) on motion");
HomieSetting<const char*> dayColor("dayColor", "color to show at day");
HomieSetting<const char*> nightColor("nightColor", "color to show at night");
HomieSetting<long> dayPercent("dayPerc", "dim white light to x% at day");
HomieSetting<long> nightPercent("nightPerc", "dim white light to x% at night");
HomieSetting<long> nightStartHour("nightStart", "Hour when night starts (0-23)");
HomieSetting<long> nightEndHour("nightEnd", "Hour when night ends (0-23)");
HomieSetting<long> minimumActivation("minimumAct", "Activation in seconds (1-999)");
HomieSetting<const char *> ntpServer("ntpServer", "NTP server (pool.ntp.org as default)");

#endif /*  LIGHT_CONFIGURATION_H */