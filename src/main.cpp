#include <Arduino.h>

#include <Homie.h>
#include <Adafruit_NeoPixel.h>
#include "ColorUtil.h"

#define firmwareVersion "0.1.1"

#define NUMBER_LEDS 6

#define BLINK_INTERVAL  500 /**< Milliseconds */
#define RESET_TRIGGER   2048

Adafruit_NeoPixel pixels(NUMBER_LEDS, D1, NEO_GRB + NEO_KHZ800);

HomieNode ledNode("strip", "Strip", "strip", true, 1, NUMBER_LEDS);
HomieNode oneLedNode /* to rule them all */("led", "Light", "led");
HomieNode lampNode("lamp", "Lamp", "WhiteLED");

bool mHomieConfigured = false;
unsigned long mLastLedChanges = 0U;
bool somethingReceived = false;

unsigned int mCount = 0;

void loopHandler() {

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

  pixels.clear();  // Initialize all pixels to 'off'
  for( int i = 0; i < NUMBER_LEDS; i++ ) {
      pixels.setPixelColor(i, pixels.ColorHSV(65535 * hue / 360, 255 * satu / 100, 255 * bright / 100));
  }
  pixels.show();   // make sure it is visible
  return true;
}


bool lightOnHandler(const HomieRange& range, const String& value) {
  if (!range.isRange) return false;  // if it's not a range

  if (range.index < 1 || range.index > NUMBER_LEDS) return false;  // if it's not a valid range

  somethingReceived = true; // Stop animation

  pixels.clear();  // Initialize all pixels to 'off'
  if (value == "off" || value == "Off" || value == "OFF") {
      pixels.setPixelColor(range.index - 1, pixels.ColorHSV(0, 0, 0));
  } else {
    /* Parse the color */
    int sep1 = value.indexOf(',');
    int sep2 = value.indexOf(',', sep1 + 1);
    int hue = value.substring(0,sep1).toInt(); /* OpenHAB  hue (0-360°) */
    int satu = value.substring(sep1 + 1, sep2).toInt(); /* OpenHAB saturation (0-100%) */
    int bright = value.substring(sep2 + 1, value.length()).toInt(); /* brightness (0-100%) */
    pixels.setPixelColor(range.index - 1, pixels.ColorHSV(65535 * hue / 360, 
                                                          255 * satu / 100, 
                                                          255 * bright / 100));
  }
  pixels.show();   // make sure it is visible

  ledNode.setProperty("led").setRange(range).send(value);  // Update the state of the led
  Homie.getLogger() << "Led " << range.index << " is " << value << endl;

  return true;
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  Homie_setFirmware("light", firmwareVersion);
  Homie.setLoopFunction(loopHandler);
  ledNode.advertise("led").settable(lightOnHandler);
  oneLedNode.advertise("ambient").setName("All Leds")
                            .setDatatype("color").settable(allLedsHandler);
  lampNode.advertise("value").setName("Value")
                                      .setDatatype("boolean")
                                      .settable(switchHandler);
               
  pixels.begin();
  pixels.clear();

  /* Set everything to red on start */
  for( int i = 0; i < NUMBER_LEDS; i++ ) {
      pixels.setPixelColor(i, 0 /*red */, 20 /* green */, 0 /* blue */);
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
      for( int i = 0; i < NUMBER_LEDS; i++ ) {
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
    }
  }

  // Use Flash button to reset configuration
  if (digitalRead(D0) == HIGH) {
    if (Homie.isConfigured()) {
      if (mCount > RESET_TRIGGER) {
        /* shutoff the LEDs */
        for( int i = 0; i < NUMBER_LEDS; i++ ) {
            pixels.setPixelColor(i, pixels.Color(0 /*red */, 0 /* green */, 0 /* blue */));
        }
        pixels.show();   // make sure it is visible
        Serial << "Delete Configuration" << endl;
        SPIFFS.format();
        SPIFFS.end();
      } else {
        uint8_t position = (uint8_t) (mCount * 255 / RESET_TRIGGER);
        RainbowCycle(&pixels, &position);
        Serial << mCount << "/" << RESET_TRIGGER << " to reset" << endl;
        mCount++;
      }
    } 
  } else {
    if (mCount > 0) {
      /* shutoff the LEDs */
      for( int i = 0; i < NUMBER_LEDS; i++ ) {
            pixels.setPixelColor(i, pixels.Color(0 /*red */, 0 /* green */, 0 /* blue */));
        }
        pixels.show();   // make sure it is visible
      mCount = 0;
    }
  }

}