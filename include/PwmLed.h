/**
 * @file PwmLed.h
 * @author Ollo
 * @brief 
 * @version 0.1
 * @date 2021-11-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef IOLIGHT_PWM_LED
#define IOLIGHT_PWM_LED

#define PWM_MAXVALUE        1023    /**< Maximum value of PWM driver, used in ESP8266 */
#define PWM_HOMIEMAXVALUE   "1023"

#define PWM_LED_DIM_TARGET_OFF -1

class PwmLED
{

private:
    /* settings */
    int mOutputPin = 0;   /**< Pin of the pump */
    int mCycleTime = 0;
    int mStep = 0;
    /* local variables */
    int mDimTarget = PWM_LED_DIM_TARGET_OFF;
    int mDimValue = PWM_LED_DIM_TARGET_OFF;
    unsigned long mLastLEDupdate = 0;
    
public:
    
    /**
     * @brief Construct a new PWM LED
     * 
     * @param pin Pin of LED
     * @param cycleTime cycle time in ms, when output is updated
     */
    PwmLED(int pinSensor, int cycleTime, int step);

    /**
     * @brief 
     * update the LED
     */
    void loop(void);

    /**
     * @brief Control PWM output
     * Directly used all possible values (0-100 is only 10% of the possible values)
     * @param targetValue 0 to @see PWM_MAXVALUE
     */
    void setPercent(int targetValue);

    /**
     * @brief Get the Percent the LEDs will fade, to
     * 
     * @return int 0 to @see PWM_MAXVALUE
     */
    int  getPercent(void);

    /**
     * @brief Get the Current PWM value
     * This will be updated during the fading procedure
     * @return int 0 to @see PWM_MAXVALUE
     */
    int  getCurrentPwm(void);

    bool isActivated(void);

    /**
     * @brief Activate fill brightness, without fading
     * 
     */
    void setOff(void);

    /**
     * @brief Deactivate all LEDs, without fading
     * 
     */
    void setOn(void);
};


#endif