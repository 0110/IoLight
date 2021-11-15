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
    static int oddCalled = 0;
    /* Skip, if the next cycle is not already */
    if (millis() < (mLastLEDupdate + (this->mCycleTime)) ) {
        return;
    }

    if (this->mDimTarget > PWM_LED_DIM_TARGET_OFF) {
        int pwmNewVal = analogRead(this->mOutputPin);
        int pwmValDiff = mDimTarget - pwmNewVal;

        /* FIXME 
        oddCalled = (oddCalled + 1) % 10;
        if ((oddCalled == 0) && mConnected) { // Update MQTT only every second call
          dimmNode.setProperty("value").send(String(((pwmVal * 100U) / PWM_MAXVALUE)));
        }*/


        /* Fade in the white light up to maximum 100% */
        if (pwmValDiff > 0) {
            pwmNewVal += this->mStep;
        } else {
            /* Fade in the white light down to maximum 0% */
            pwmNewVal -= this->mStep;
        }

        if ((abs(pwmValDiff) > 0) || (abs(pwmValDiff) > 0)) {
            if (pwmNewVal < 0) {
                analogWrite(this->mOutputPin, 0);
                this->mDimTarget = PWM_LED_DIM_TARGET_OFF;
            } else if(pwmNewVal > PWM_MAXVALUE) {
                analogWrite(this->mOutputPin, PWM_MAXVALUE);
                this->mDimTarget = PWM_LED_DIM_TARGET_OFF;
            } else {
                analogWrite(this->mOutputPin, pwmNewVal); 
            }
        } else {
            this->mDimTarget = PWM_LED_DIM_TARGET_OFF;
        }

    }   
}

bool PwmLED::isActivated(void) {
    return (analogRead(this->mOutputPin) > 0);
}

void PwmLED::dimPercent(int targetValue) {
    if ((targetValue >= 0) && (targetValue <= 100)) {
        this->mDimTarget = ((targetValue * PWM_MAXVALUE) / 100U);
    }
}

void PwmLED::setOff(void) {
  analogWrite(this->mOutputPin, 0); // activate LED with 0%
}

void PwmLED::setOn(void) {
    analogWrite(this->mOutputPin, PWM_MAXVALUE); // activate LED with 0%
}
