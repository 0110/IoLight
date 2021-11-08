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

class PwmLED
{

private:
    int mOutputPin = 0;   /**< Pin of the pump */
    
public:
    
    /**
     * @brief Construct a new PWM LED
     * 
     * @param pin Pin of LED
     * @param cycleTime cycle time in ms, when output is updated
     */
    PwmLED(int pinSensor, int cycleTime);

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
    void dim(int targetValue);

    void setOff(void);
    void setOn(void);

};


#endif