#include <Arduino.h>

#include <Homie.h>
#include <Adafruit_NeoPixel.h>

#define firmwareVersion "0.1.0"

#define NUMBER_OF_LED 6

Adafruit_NeoPixel pixels(NUMBER_OF_LED, D1, NEO_GRB + NEO_KHZ800);

HomieNode ledNode("strip", "Strip", "strip", true, 1, NUMBER_OF_LED);
bool mHomeConfigured = false;
unsigned long mLastLedChanges = 0U;

bool lightOnHandler(const HomieRange& range, const String& value) {
  if (!range.isRange) return false;  // if it's not a range

  if (range.index < 1 || range.index > NUMBER_OF_LED) return false;  // if it's not a valid range

  if (value != "on" && value != "off") return false;  // if the value is not valid

  ledNode.setProperty("led").setRange(range).send(value);  // Update the state of the led
  Homie.getLogger() << "Led " << range.index << " is " << value << endl;

  return true;
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  Homie_setFirmware("light", firmwareVersion);
  ledNode.advertise("color").settable(lightOnHandler);
  pixels.begin();
  pixels.clear();
  Homie.setup();

  mHomeConfigured = Homie.isConfigured();
}

void loop() {
  Homie.loop();
  if (!mHomeConfigured) {
    if ((millis() - mLastLedChanges) < 1000)
    {
      for(int i=0; i < NUMBER_OF_LED; i++) {
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      }
      pixels.show();
    } else if ( (millis() - mLastLedChanges) < 2000)
    {
      for(int i=0; i < NUMBER_OF_LED; i++) {
        pixels.setPixelColor(i, pixels.Color(0, 150, 0));
      }
      pixels.show();
    }
    else if ((millis() - mLastLedChanges) < 3000)
    {
      for(int i=0; i < NUMBER_OF_LED; i++) {
        pixels.setPixelColor(i, pixels.Color(0, 0, 150));  
      }
      pixels.show();
    }
    else if ((millis() - mLastLedChanges) < 4000)
    {
      for(int i=0; i < NUMBER_OF_LED; i++) {
        pixels.setPixelColor(i, pixels.Color(150, 0, 0));  
      }
      pixels.show();
    }
    else
    {
      mLastLedChanges = millis();
    }
  }
}