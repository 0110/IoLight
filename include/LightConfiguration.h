/**
 * @brief Light configuration
 * @file  LightConfiguration.h
 * 
 */

#ifndef LIGHT_CONFIGURATION_H
#define LIGHT_CONFIGURATION_H

#define FIRMWARE_VERSION "1.0.4"

/***************** Build firmware name according compiled features ***/
#ifndef NOBUTTON
#define FIRMWARE_FEATURE1 "WithButton"
#else
#define FIRMWARE_FEATURE1 ""
#endif
#ifdef PIR_ENABLE
#define FIRMWARE_FEATURE2 "WithPIR_"
#else
#define FIRMWARE_FEATURE2 ""
#endif
#ifdef TEMP_ENABLE
#define FIRMWARE_FEATURE3 "WithTemp"
#else
#define FIRMWARE_FEATURE3 ""
#endif
#define FIRMWARE_NAME "light" FIRMWARE_FEATURE1 FIRMWARE_FEATURE2 FIRMWARE_FEATURE3

#define NUMBER_LEDS 388

#define BLINK_INTERVAL  500 /**< Milliseconds cycle time */
#define FADE_INTERVAL   20  /**< Milliseconds cycle time */
#define RESET_TRIGGER   2048
#define PWM_STEP        20
#define FADE_MAXVALUE   255

#define TIME_UNDEFINED  0xFFFFFFFFU
#define TIME_FADE_DONE  0xFFFFFFFEU

#define HOMIE_TRUE      "true"
#define HOMIE_FALSE     "false"

#ifndef NOBUTTON
#ifdef PIR_ENABLE
#error PIR and BUTTON are both connected at same input -> select one method
#endif
#endif

#define GPIO_BUTTON     D6  /**< Input button */
#define GPIO_WS2812     D1  /**< RGB LEDs */
#define GPIO_PIR        D6  /**< Passive infrared sensor */
#define GPIO_LED        D2  /**< None RGB light, controlled by mosfet */
#define GPIO_DS18B20    D7  /**< One-Wire used for Dallas temperature sensor */

#define LEVEL_ERROR             1
#define LEVEL_WARNING           2
#define LEVEL_INFO              3
#define LEVEL_DEBUG             4
#define LEVEL_LOG               5
#define STATUS_HARDWARE_BUTTON    100
#define STATUS_MQTT_DETECTED    100
#define STATUS_MOTION_DETECTED  1000
#define STATUS_MOTION_CHANGED    999
#define STATUS_PWM_STARTS       1001
#define STATUS_PWM_INITIAL      1002
#define STATUS_PWM_RETRIGGER    1003
#define STATUS_PWM_FINISHED     1004
#define STATUS_PWM_UPDATE       1005
#define STATUS_UNKNOWN_CMD      9000

HomieSetting<long> ledAmount("leds", "Amount of LEDs (of type WS2812); Range 1 to 2047");
#ifdef TEMP_ENABLE
HomieSetting<bool> oneWireSensorAvail("oneWire", "One wire Bus installed at D7 (disabled as default)");
#endif
#ifdef PIR_ENABLE
#define     HOMIE_MAXPERCENT    100
HomieSetting<const char*> dayColor("dayColor", "color to show at day");
HomieSetting<const char*> nightColor("nightColor", "color to show at night");
HomieSetting<long> dayPercent("dayPerc", "dim white light to x% at day (0: disabled)");
HomieSetting<long> nightPercent("nightPerc", "dim white light to x% at night (0: disabled)");
HomieSetting<long> nightStartHour("nightStart", "Hour when night starts (0-23)");
HomieSetting<long> nightEndHour("nightEnd", "Hour when night ends (0-23)");
HomieSetting<long> minimumActivation("minimumAct", "Activation in seconds (1-999)");
HomieSetting<const char *> ntpServer("ntpServer", "NTP server (pool.ntp.org as default)");
#endif

#endif /*  LIGHT_CONFIGURATION_H */