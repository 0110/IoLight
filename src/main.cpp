/**
 * @file main.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2020-01-17
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef UNIT_TEST
#include <Arduino.h>

#include <Homie.h>
#include <Adafruit_NeoPixel.h>
#include "LightConfiguration.h"
#include "ColorUtil.h"
#include <time.h>  

Adafruit_NeoPixel* pPixels = NULL;

HomieNode ledNode("strip", "Strip", "strip", true, 1, NUMBER_LEDS);
HomieNode oneLedNode /* to rule them all */("led", "Light", "led");
HomieNode lampNode("lamp", "Lamp", "WhiteLED");
HomieNode motion("motion", "motion", "Motion detected");

bool mHomieConfigured = false;
unsigned long mLastLedChanges = 0U;
bool somethingReceived = false;

unsigned int mCount = 0;
bool mLastMotion=false;

void onHomieEvent(const HomieEvent &event)
{
  switch (event.type)
  {
  case HomieEventType::SENDING_STATISTICS:
    Homie.getLogger() << "My statistics" << endl;
    break;
  case HomieEventType::MQTT_READY:
    Serial.printf("NTP Setup with server %s\r\n", ntpServer.get());
    configTime(0, 0, ntpServer.get());
  default:
    break;
  }
}

void loopHandler() {
  // Handle motion sensor
  if (mLastMotion != digitalRead(D6)) {
    // Read the current time
    time_t now; // this is the epoch
    tm tm;      // the structure tm holds time information in a more convient way
    time(&now);
    localtime_r(&now, &tm);
    // Update the motion state
    mLastMotion = digitalRead(D6);

    Serial << "Motion: " << mLastMotion << " at " << (1900 + tm.tm_year) << "-" << (tm.tm_mon + 1) << "-" << tm.tm_mday << " " << tm.tm_hour << ":" << tm.tm_min << ":" << tm.tm_sec << endl;
    motion.setProperty("motion").send(String(mLastMotion ? "true" : "false"));

    /* Tetermine color according to time (night / day) */
    //FIXME: extractColor(candidate, strlen(candidate))

    if (tm.tm_year < 100){ /* < 2000 as tm_year + 1900 is the year */
      return;
    }

    uint32_t color = extractColor(dayColor.get(), strlen(dayColor.get()) );
    if ((nightStartHour.get() <= tm.tm_hour) || (tm.tm_hour <= nightEndHour.get()) ) {
        color = extractColor(nightColor.get(), strlen(nightColor.get()) );
    }

    somethingReceived = true;
    /* Set everything to red on start */
    for( int i = 0; i < ledAmount.get(); i++ ) {
        if (mLastMotion) {
          pPixels->setPixelColor(i, color);
        } else {
          pPixels->setPixelColor(i, 0);
        }
    }
    pPixels->show();
  }

  // Feed the dog -> ESP stay alive
  ESP.wdtFeed();
}

bool switchHandler(const HomieRange& range, const String& value) {
  if (range.isRange) return false;  // only one switch is present
  
  //FIXME add the output of the white lamp
  return true;
}

bool allLedsHandler(const HomieRange& range, const String& value) {
  if (range.isRange) return false;  // only one switch is present

  somethingReceived = true; // Stop animation

  int sep1 = value.indexOf(',');
  int sep2 = value.indexOf(',', sep1 + 1);
  /*FIXME add here the code to change the color */
  int hue = value.substring(0,sep1).toInt(); /* OpenHAB  hue (0-360°) */
  int satu = value.substring(sep1 + 1, sep2).toInt(); /* OpenHAB saturation (0-100%) */
  int bright = value.substring(sep2 + 1, value.length()).toInt(); /* brightness (0-100%) */

  if (pPixels) {
    pPixels->clear();  // Initialize all pixels to 'off'
    for( int i = 0; i < ledAmount.get(); i++ ) {
        pPixels->setPixelColor(i, pPixels->ColorHSV(65535 * hue / 360, 255 * satu / 100, 255 * bright / 100));
    }
    pPixels->show();   // make sure it is visible
  }
  return true;
}


bool lightOnHandler(const HomieRange& range, const String& value) {
  if (!range.isRange) return false;  // if it's not a range

  if (range.index < 1 || range.index > ledAmount.get()) return false;  // if it's not a valid range

  somethingReceived = true; // Stop animation

  if (pPixels == NULL) {
    return false;
  }

  pPixels->clear();  // Initialize all pixels to 'off'
  if (value == "off" || value == "Off" || value == "OFF") {
      pPixels->setPixelColor(range.index - 1, pPixels->ColorHSV(0, 0, 0));
  } else {
    /* Parse the color */
    pPixels->setPixelColor(range.index - 1, extractColor(value.c_str(), value.length()));
  }
  pPixels->show();   // make sure it is visible

  ledNode.setProperty("led").setRange(range).send(value);  // Update the state of the led
  Homie.getLogger() << "Led " << range.index << " is " << value << endl;

  return true;
}

void setup() {
  /* Load Filesystem */
  SPIFFS.begin();
  Serial.begin(115200);
  Serial << endl << endl;
  Homie_setFirmware("light", FIRMWARE_VERSION);
  Homie.setLoopFunction(loopHandler);
  Homie.onEvent(onHomieEvent);
  ledNode.advertise("led").setName("Each Leds").setDatatype("color").settable(lightOnHandler);
  motion.advertise("motion").setName("Motion").setDatatype("boolean");
  oneLedNode.advertise("ambient").setName("All Leds")
                            .setDatatype("color").settable(allLedsHandler);
  lampNode.advertise("value").setName("Value")
                                      .setDatatype("boolean")
                                      .settable(switchHandler);

  // Load the settings and  set default values
  ledAmount.setDefaultValue(NUMBER_LEDS).setValidator([] (long candidate) {
    return (candidate > 0) && (candidate < 2048);
  });
  motionActivation.setDefaultValue(false);
  dayColor.setDefaultValue("off").setValidator([] (const char *candidate) {
    return extractColor(candidate, strlen(candidate)) != 0xFFFFFFFF;
  });
  nightColor.setDefaultValue("red").setValidator([] (const char *candidate) {
    return extractColor(candidate, strlen(candidate)) != 0xFFFFFFFF;
  });
  nightStartHour.setDefaultValue(22).setValidator([] (long candidate) {
    return (candidate >= 0) && (candidate < 24);
  });
  nightEndHour.setDefaultValue(6).setValidator([] (long candidate) {
    return (candidate >= 0) && (candidate < 24);
  });
  minimumActivation.setDefaultValue(1).setValidator([] (long candidate) {
    return (candidate >= 0) && (candidate < 1000);
  });
  ntpServer.setDefaultValue("pool.ntp.org");


  pPixels = new Adafruit_NeoPixel(ledAmount.get(), D1, NEO_GRB + NEO_KHZ800);

  pPixels->begin();
  pPixels->clear();

  /* Set everything to red on start */
  for( int i = 0; i < ledAmount.get(); i++ ) {
      pPixels->setPixelColor(i, 0 /*red */, 20 /* green */, 0 /* blue */);
  }
  pPixels->show();

  Homie.setup();
  pinMode(D0, INPUT); // GPIO0 as input
  mHomieConfigured = Homie.isConfigured();
  pinMode(D6, INPUT);
}

void loop() {
  Homie.loop();
  if (!mHomieConfigured) {
    if ( ((millis() - mLastLedChanges) >= BLINK_INTERVAL) ||
        (mLastLedChanges == 0) ) {
      int blue = 128;
      // set the colors for the strip
      if (pPixels->getPixelColor(0) > 0) {
        blue = 0;
      }
      for( int i = 0; i < ledAmount.get(); i++ ) {
          pPixels->setPixelColor(i, 0 /*red */, 0 /* green */, blue /* blue */);
      }
      pPixels->show();
      mLastLedChanges = millis();    
    }

  } else if (!somethingReceived) {
    static uint8_t position = 0;
    if ( ((millis() - mLastLedChanges) >= 20) ||
        (mLastLedChanges == 0) ) {
      RainbowCycle(pPixels, &position);
      mLastLedChanges = millis();    
    }
  }

  // Use Flash button to reset configuration
  if (digitalRead(D0) == HIGH) {
    if (Homie.isConfigured()) {
      if (mCount > RESET_TRIGGER) {
        /* shutoff the LEDs */
        for( int i = 0; i < ledAmount.get(); i++ ) {
            pPixels->setPixelColor(i, pPixels->Color(0 /*red */, 0 /* green */, 0 /* blue */));
        }
        pPixels->show();   // make sure it is visible
        if (SPIFFS.exists ("/homie/config.json") ) 
        { 
          Serial << "Delete Configuration" << endl;
          SPIFFS.remove("/homie/config.json");
        }
        SPIFFS.end();
      } else {
        uint8_t position = (uint8_t) (mCount * 255 / RESET_TRIGGER);
        RainbowCycle(pPixels, &position);
        Serial << mCount << "/" << RESET_TRIGGER << " to reset" << endl;
        mCount++;
      }
    } 
  } else {
    if (mCount > 0) {
      /* shutoff the LEDs */
      for( int i = 0; i < ledAmount.get(); i++ ) {
            pPixels->setPixelColor(i, pPixels->Color(0 /*red */, 0 /* green */, 0 /* blue */));
        }
        pPixels->show();   // make sure it is visible
      mCount = 0;
    }
  }

}
#endif