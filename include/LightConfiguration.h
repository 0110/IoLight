/**
 * @brief Light configuration
 * @file  LightConfiguration.h
 * 
 */

#ifndef LIGHT_CONFIGURATION_H
#define LIGHT_CONFIGURATION_H


#define FIRMWARE_VERSION "0.2.1"

#define NUMBER_LEDS 6

#define BLINK_INTERVAL  500 /**< Milliseconds */
#define RESET_TRIGGER   2048
#define PWM_MAXVALUE    1023
#define FADE_MAXVALUE   255

HomieSetting<long> ledAmount("leds", "Amount of LEDs (of type WS2812); Range 1 to 2047");
HomieSetting<bool> motionActivation("motionactivation", "Activate light on motion");
HomieSetting<const char*> dayColor("dayColor", "color to show at day");
HomieSetting<const char*> nightColor("nightColor", "color to show at night");
HomieSetting<long> nightStartHour("nightStart", "Hour when night starts (0-23)");
HomieSetting<long> nightEndHour("nightEnd", "Hour when night ends (0-23)");
HomieSetting<long> minimumActivation("minimumAct", "Activation in seconds (1-999)");
HomieSetting<const char *> ntpServer("ntpServer", "NTP server (pool.ntp.org as default)");

#endif /*  LIGHT_CONFIGURATION_H */