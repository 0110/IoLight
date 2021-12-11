/**
 * @file Ollo
 * @brief 
 * @version 0.1
 * @date 2021-11-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "PwmLed.h"
#include <Arduino.h>

PwmLED::PwmLED(int pinSensor, int cycleTime, int step) {
    this->mOutputPin = pinSensor;
    this->mCycleTime = cycleTime;
    this->mStep = step;
    pinMode(this->mOutputPin, OUTPUT); // PWM Pin for white LED
}

void PwmLED::loop(void) {
    /* Skip, if the next cycle is not already */
    if (millis() < (mLastLEDupdate + (this->mCycleTime)) ) {
        return;
    }

    if (this->mDimTarget > PWM_LED_DIM_TARGET_OFF) {
        int pwmNewVal = mDimValue;
        int pwmValDiff = mDimTarget - pwmNewVal;

        /* Fade in the white light up to maximum 100% */
        if (pwmValDiff > 0) {
            pwmNewVal += this->mStep;
        } else {
            /* Fade in the white light down to maximum 0% */
            pwmNewVal -= this->mStep;
        }

        if ((pwmValDiff > PWM_MINSTEP) || (pwmValDiff < (-PWM_MINSTEP))) {
            if (pwmNewVal < 0) {
                mDimValue = 0;
            } else if(pwmNewVal > PWM_MAXVALUE) {
                mDimValue = PWM_MAXVALUE;
            } else {
                mDimValue = pwmNewVal;
            }
            analogWrite(this->mOutputPin, mDimValue); 
            mLastLEDupdate = millis();
        } else if ((mDimValue > 0) && (pwmNewVal < PWM_MINSTEP)) {
            /* close to shutdown; deactivate the light completly */
            mDimValue = 0;
            analogWrite(this->mOutputPin, mDimValue); 
        } else {
            this->mDimTarget = PWM_LED_DIM_TARGET_OFF;
        }
    }   
}

bool PwmLED::isActivated(void) {
    return (mDimValue > 0);
}

void PwmLED::setPercent(int targetValue) {
    if ((targetValue >= 0) && (targetValue <= 100)) {
        this->mDimTarget = ((targetValue * PWM_MAXVALUE) / 100U);
    }
}

int PwmLED::getPercent(void) {
    return (100UL * (this->mDimTarget) ) / PWM_MAXVALUE;
}

int PwmLED::getCurrentPwm(void) {
    return mDimValue;
}

void PwmLED::setOff(void) {
  mDimValue = 0; // activate LED with 0%
  analogWrite(this->mOutputPin, mDimValue);
}

void PwmLED::setOn(void) {
    mDimValue = PWM_MAXVALUE; // activate LED with 0%
    analogWrite(this->mOutputPin, mDimValue);
}
