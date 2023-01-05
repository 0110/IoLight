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

#define PWM_MAXVALUE    1023
#define PWM_MINSTEP     3

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
     * @brief 
     * 
     * @param targetValue 
     */
    void setPercent(int targetValue);
    int  getPercent(void);
    int  getCurrentPwm(void);
    bool isActivated(void);

    void setOff(void);
    void setOn(void);
};


#endif