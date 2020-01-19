#include <Arduino.h>

#include <Homie.h>
#include <Adafruit_NeoPixel.h>
#include "ColorUtil.h"

#define firmwareVersion "0.1.1"

#define NUMBER_OF_LED 6

#define BLINK_INTERVAL  500 /**< Milliseconds */

Adafruit_NeoPixel pixels(NUMBER_OF_LED, D1, NEO_GRB + NEO_KHZ800);

HomieNode ledNode("strip", "Strip", "strip", true, 1, NUMBER_OF_LED);
bool mHomieConfigured = false;
unsigned long mLastLedChanges = 0U;
bool somethingReceived = false;

void loopHandler() {
  //TODO add here some logic, that is triggered when Homie is configured
}

bool lightOnHandler(const HomieRange& range, const String& value) {
  if (!range.isRange) return false;  // if it's not a range

  if (range.index < 1 || range.index > NUMBER_OF_LED) return false;  // if it's not a valid range

  somethingReceived = true; // Stop animation

  if (value != "on" && value != "off") return false;  // if the value is not valid

  ledNode.setProperty("led").setRange(range).send(value);  // Update the state of the led
  Homie.getLogger() << "Led " << range.index << " is " << value << endl;

  return true;
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  Homie_setFirmware("light", firmwareVersion);
  Homie.setLoopFunction(loopHandler);
  ledNode.advertise("color").settable(lightOnHandler);
  pixels.begin();
  pixels.clear();

  /* Set everything to red on start */
  for( int i = 0; i < NUMBER_OF_LED; i++ ) {
      pixels.setPixelColor(i, 20 /*red */, 0 /* green */, 0 /* blue */);
  }
  pixels.show();

  Homie.setup();
  pinMode(D0, INPUT); // GPIO0 as input
  mHomieConfigured = Homie.isConfigured();
}

void loop() {
  Homie.loop();
  if (!mHomieConfigured) {
    if ( ((millis() - mLastLedChanges) >= BLINK_INTERVAL) ||
        (mLastLedChanges == 0) ) {
      int blue = 128;
      // set the colors for the strip
      if (pixels.getPixelColor(0) > 0) {
        blue = 0;
      }
      for( int i = 0; i < NUMBER_OF_LED; i++ ) {
          pixels.setPixelColor(i, 0 /*red */, 0 /* green */, blue /* blue */);
      }
      pixels.show();
      mLastLedChanges = millis();    
    }
  } else if (!somethingReceived) {
    static uint8_t position = 0;
    if ( ((millis() - mLastLedChanges) >= 20) ||
        (mLastLedChanges == 0) ) {
      RainbowCycle(&pixels, &position);
      mLastLedChanges = millis();    
      Serial << "." << position << endl;
    }
  }

  // Use Flash button to reset configuration
  if (digitalRead(D0) == HIGH) {
    if (Homie.isConfigured()) {
      Serial << "Delete Configuration" << endl;
      SPIFFS.begin();
      SPIFFS.format();
      SPIFFS.end();
    } else {
      Serial << "GPPIO 0 pressed" << endl;
    }
  }

}