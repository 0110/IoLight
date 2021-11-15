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
#include "DallasTemperature.h"
#include <OneWire.h>
#include "PwmLed.h"

/******************************************************************************
 *                                     DEFINES
 ******************************************************************************/
#define TEMP_SENSOR_MEASURE_SERIES  5 /**< Maximum resets */
#define MIN_TEMP_DIFFERENCE   0.1f
#define NODE_TEMPERATUR "degrees"
#define LOG_TOPIC "log\0"

#define getTopic(test, topic)                                                                                                                 \
  char *topic = new char[strlen(Homie.getConfiguration().mqtt.baseTopic) + strlen(Homie.getConfiguration().deviceId) + 1 + strlen(test) + 1]; \
  strcpy(topic, Homie.getConfiguration().mqtt.baseTopic);                                                                                     \
  strcat(topic, Homie.getConfiguration().deviceId);                                                                                           \
  strcat(topic, "/");                                                                                                                         \
  strcat(topic, test);

/******************************************************************************
 *                                     MACROS
 ******************************************************************************/
#define DIFFERENCE(a,b)   ( ((a) > (b)) ? ((a) - (b)) : ((b) - (a)) )

/******************************************************************************
 *                            FUNCTION PROTOTYPES
 ******************************************************************************/

void log(int level, String message, int code);

/******************************************************************************
 *                            LOCAL VARIABLES
 ******************************************************************************/
Adafruit_NeoPixel* pPixels = NULL;
OneWire oneWire(GPIO_DS18B20);
PwmLED led(GPIO_LED, FADE_INTERVAL, PWM_STEP);
DallasTemperature sensors(&oneWire);
float mLastTemperatur = -10.0f;

HomieNode ledNode("strip", "Strip", "strip", true, 1, NUMBER_LEDS);
HomieNode oneLedNode /* to rule them all */("led", "RGB led", "all leds");
HomieNode lampNode("lamp", "Lamp switch", "White lamp On-Off");
HomieNode dimmNode("dimm", "Lamp Dimmed", "White lamp can be dimmed");
HomieNode monitor("monitor", "Monitor motion", "Monigor motion via PIR");
HomieNode temperatureNode("temperature", "Temparture", "inside case");

bool mHomieConfigured = false;
unsigned long mLastLedChanges = 0U;
bool somethingReceived = false;

unsigned int mButtonPressingCount = 0;        /**< Delay before everything is reset */
unsigned int mColorFadingCount = FADE_MAXVALUE;
bool mLastMotion=false;
unsigned long mShutoffAfterMotion = TIME_UNDEFINED;     /**< Time, when LED has to be deactivated after motion */
bool mConnected = false;

/******************************************************************************
 *                            LOCAL FUNCTIONS
 ******************************************************************************/

void onHomieEvent(const HomieEvent &event)
{
  switch (event.type)
  {
  case HomieEventType::SENDING_STATISTICS:
    Homie.getLogger() << "My statistics" << endl;
    break;
  case HomieEventType::MQTT_READY:
#ifdef PIR_ENABLE
    Serial.printf("NTP Setup with server %s\r\n", ntpServer.get());
    configTime(0, 0, ntpServer.get());
#endif
    mConnected = true;
  default:
    break;
  }
}

void loopHandler() {

#ifdef PIR_ENABLE
  // Handle motion sensor
  int readMotion = digitalRead(GPIO_PIR);
  if (mLastMotion != readMotion) {
    // Read the current time
    time_t now; // this is the epoch
    tm tm;      // the structure tm holds time information in a more convient way
    time(&now);
    localtime_r(&now, &tm);
    // Update the motion state
    mLastMotion = readMotion;
    log(LEVEL_DEBUG, String("Motion : ") + String(mLastMotion ? "high" : "low") ,STATUS_MOTION_CHANGED);
    
    if (!mConnected || (tm.tm_year < 100)) { /* < 2000 as tm_year + 1900 is the year */
      return;
    }
    monitor.setProperty("motion").send(String(mLastMotion ? "true" : "false"));
    
    if ((mLastMotion == HIGH) && 
        (mShutoffAfterMotion == TIME_FADE_DONE)) {
      
      /* clear LEDs */
      if (!somethingReceived) {
        pPixels->clear();
        pPixels->show();
        led.setPercent(0);
      }
      somethingReceived = true; // Stop the animation, as a montion was detected */
      

      log(LEVEL_DEBUG, String(mShutoffAfterMotion, 16) + String(" ") +
          String("Fade" + String(mColorFadingCount) + " Time: " + millis() + " left: " + String((mShutoffAfterMotion- millis())/1000) + String("s")), STATUS_MOTION_DETECTED);
      
      uint32_t color = extractColor(dayColor.get(), strlen(dayColor.get()) );
      int maxPercent = dayPercent.get();
      if ((nightStartHour.get() <= tm.tm_hour) || (tm.tm_hour <= nightEndHour.get()) ) {
          color = extractColor(nightColor.get(), strlen(nightColor.get()) );
          maxPercent = nightPercent.get();
          oneLedNode.setProperty("ambient").send(String(nightColor.get()));
      } else {
        oneLedNode.setProperty("ambient").send(String(dayColor.get()));
      }

      // check, if dayColor or nightcolor has the value "Decativated"
      if (color == 0U) {
        mColorFadingCount = 0;  
      } else {
        /* Activate everything */
        mColorFadingCount = 1;
        if (maxPercent > 0) {
          led.setPercent(maxPercent);
          log(LEVEL_DEBUG,String("PWM starts (" + String(maxPercent) + "%)"), STATUS_PWM_STARTS);
        } else {
          /* At night, deactivate the white LED */
          ;
        }
        for( int i = 0; i < ledAmount.get(); i++ ) {
          pPixels->setBrightness(mColorFadingCount);
          pPixels->setPixelColor(i, color);
        }
      }

      mShutoffAfterMotion = millis() + (minimumActivation.get() * 1000);
      log(LEVEL_DEBUG,String("Update " + String(mShutoffAfterMotion) ), STATUS_PWM_RETRIGGER);
    }
  }
#endif

  if (oneWireSensorAvail.get()) {
      int sensorCount = sensors.getDeviceCount();
      if (sensorCount > 0)
      {
        sensors.requestTemperatures();
        float temp1Raw = sensors.getTempCByIndex(0);
        float change = DIFFERENCE(temp1Raw, mLastTemperatur);
        if (change > MIN_TEMP_DIFFERENCE) {
          temperatureNode.setProperty(NODE_TEMPERATUR).send(String(temp1Raw));
          Serial << "Temp: " << temp1Raw << endl;
          mLastTemperatur=temp1Raw;
        }
      }
  }

  // Feed the dog -> ESP stay alive
  ESP.wdtFeed();
}

bool switchHandler(const HomieRange& range, const String& value) {
  if (range.isRange) return false;  // only one switch is present
  if (value == "off" || value == "Off" || value == "OFF" || value == "false") {
    led.setOff();
    dimmNode.setProperty("value").send(value);
  } else if (value == "on" || value == "On" || value == "ON" || value == "true") {
    led.setOn();
    dimmNode.setProperty("value").send(value);
  } else if ( value.length() > 0 && isDigit(value.charAt(0))  ) {
      int targetVal = value.toInt();
      if ((targetVal >= 0) && (targetVal <= 100)) {
        log(LEVEL_LOG, String("MQTT | Dimm to ") + String(value.toInt()) + String( "%"), STATUS_PWM_STARTS);
        led.setPercent(targetVal);
        dimmNode.setProperty("value").send(value);
      } else {
          log(LEVEL_ERROR, String("MQTT | Unknown percent: '") + String(value) + String( "'"), STATUS_PWM_STARTS);
      }
  } else {
    log(LEVEL_ERROR, String(value), STATUS_UNKNOWN_CMD);
  }
  somethingReceived = true; // Stop animation
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
    uint8_t c = pPixels->ColorHSV(65535 * hue / 360, 255 * satu / 100, 255 * bright / 100);
    pPixels->clear();  // Initialize all pixels to 'off'
    for( int i = 0; i < ledAmount.get(); i++ ) {
        pPixels->setPixelColor(i, c);
    }
    pPixels->show();   // make sure it is visible
    if (mConnected) {
      oneLedNode.setProperty("ambient").send(String(value));
    }
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
  ledNode.advertise("led").setName("Each Leds").setDatatype("Colour").setUnit("rgb")
                            .settable(lightOnHandler);
  monitor.advertise("motion").setName("Monitor motion").setDatatype("Boolean");
  oneLedNode.advertise("ambient").setName("All Leds")
                            .setDatatype("Colour").setUnit("rgb")
                            .settable(allLedsHandler);
  lampNode.advertise("value").setName("Value")
                                      .setDatatype("Boolean")
                                      .settable(switchHandler);
  dimmNode.advertise("value").setName("Dimmer")
                                      .setDatatype("Integer")
                                      .setUnit("%")
                                      .settable(switchHandler);
  temperatureNode.advertise(NODE_TEMPERATUR).setName("Degrees")
                                      .setDatatype("Float")
                                      .setUnit("ºC");                                      


  // Load the settings and  set default values
  ledAmount.setDefaultValue(NUMBER_LEDS).setValidator([] (long candidate) {
    return (candidate > 0) && (candidate < 2048);
  });
  #ifdef PIR_ENABLE
  dayColor.setDefaultValue("off").setValidator([] (const char *candidate) {
    return extractColor(candidate, strlen(candidate)) != 0xFFFFFFFF;
  });
  nightColor.setDefaultValue("red").setValidator([] (const char *candidate) {
    return extractColor(candidate, strlen(candidate)) != 0xFFFFFFFF;
  });
  dayPercent.setDefaultValue(0).setValidator([] (long candidate) {
    return (candidate >= 0) && (candidate <= 100);
  });
  nightPercent.setDefaultValue(0).setValidator([] (long candidate) {
    return (candidate >= 0) && (candidate <= 100);
  });
  nightStartHour.setDefaultValue(22).setValidator([] (long candidate) {
    return (candidate >= 0) && (candidate < 24);
  });
  nightEndHour.setDefaultValue(6).setValidator([] (long candidate) {
    return (candidate >= 0) && (candidate < 24);
  });
  minimumActivation.setDefaultValue(30).setValidator([] (long candidate) {
    return (candidate >= 0) && (candidate < 1000);
  });
  ntpServer.setDefaultValue("pool.ntp.org");
#endif
  oneWireSensorAvail.setDefaultValue(false).setValidator([] (int candidate) {
    return true;
  });

  Homie.setup();
  mHomieConfigured = Homie.isConfigured();

  pPixels = new Adafruit_NeoPixel(ledAmount.get(), GPIO_WS2812, NEO_GRB + NEO_KHZ800);

  pPixels->begin();
  pPixels->clear();

  /* Set everything to red on start */
  for( int i = 0; i < ledAmount.get(); i++ ) {
      pPixels->setPixelColor(i, 0 /*red */, 20 /* green */, 0 /* blue */);
  }
  pPixels->show();
  Serial << "WS2812 Strip initialized with " << (ledAmount.get()) << " leds" << endl;
#ifndef NOBUTTON
  pinMode(GPIO_BUTTON, INPUT); // GPIO0 as input
#endif 
#ifdef PIR_ENABLE
  pinMode(GPIO_PIR, INPUT);
#endif 
  led.setPercent(100);
  if (oneWireSensorAvail.get()) {
    sensors.begin();
          for(int j=0; j < TEMP_SENSOR_MEASURE_SERIES && sensors.getDeviceCount() == 0; j++) {
        delay(100);
        sensors.begin();
        Serial << "Reset 1-Wire Bus" << endl;
      }
  }
  // always shutdown LED after controller was started
  mShutoffAfterMotion = millis() + (minimumActivation.get() * 1000);
}

void loop() {
  Homie.loop();
  led.loop();
  /* Chip is not configured */
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

  /* No input, chip is in IDLE mode */
  } else if (!somethingReceived) {
    static uint8_t position = 0;
    if ( ((millis() - mLastLedChanges) >= FADE_INTERVAL) ||
        (mLastLedChanges == 0U) ) {
      RainbowCycle(pPixels, &position);
      position++;
      if (millis() > mShutoffAfterMotion) {
        log(LEVEL_DEBUG,String("Initial finished to ") + String(mShutoffAfterMotion, 16) + String("s"), STATUS_PWM_FINISHED);
        led.setPercent(0);
        mShutoffAfterMotion = TIME_FADE_DONE;
      }
      mLastLedChanges = millis();
    }
  /* the chip has to do something with color */
  } else {
    if ((millis() - mLastLedChanges) >= FADE_INTERVAL) {
      /* LEDs are in the configured time frame, where they must be activated */
      if (millis() < mShutoffAfterMotion) {
        if ((mColorFadingCount > 0) && (mColorFadingCount < FADE_MAXVALUE)) {
          mColorFadingCount+=2;
          pPixels->setBrightness(mColorFadingCount);
          pPixels->show();
        }
      } else {
        /* enough enlightment... deactivate */
        if (led.isActivated() ||
              (pPixels->getBrightness() > 0) ) {
          log(LEVEL_INFO, String("Time gone: ") + String((millis() - mShutoffAfterMotion) / 1000) + String("s ") + 
                          String(pPixels->getBrightness()) + String("% ") +
                          String(led.isActivated()) + String(" PWM activated:") +
                          String(led.getPercent()) + String("%")
                          , STATUS_PWM_FINISHED);
          
          /* shutdown again */
          led.setPercent(0);
                    
          pPixels->fill(pPixels->Color(0,0,0));
          pPixels->setBrightness(0);
          pPixels->show();
        } else {
          log(LEVEL_DEBUG,String("Ready after ") + String(mShutoffAfterMotion / 1000) + String("s"), STATUS_PWM_FINISHED);
          mShutoffAfterMotion = TIME_FADE_DONE;
        }
      }
      mLastLedChanges = millis();
    }
  }

  // check serial
  if (Serial.available()) {
    int c = Serial.read();
    switch (c)
    {
    case 'c':
      if (mHomieConfigured) {
        if (SPIFFS.exists ("/homie/config.json") ) 
        { 
          Serial << "Delete Configuration" << endl;
          SPIFFS.remove("/homie/config.json");
        }
      } else {
        Serial.println("Clear config not possible");
      }
      break;
    
    default:
      Serial.print("Unknown command: ");
      Serial.println((char) c);
      Serial.println("Usage: ");
      Serial.println("c - clear homie configuration");
      break;
    }

  }
#ifndef NOBUTTON
  // Use Flash button to reset configuration
  if (digitalRead(GPIO_BUTTON) == HIGH) {
    if (mHomieConfigured) {
      if (mButtonPressingCount > RESET_TRIGGER) {
        /* shutoff the LEDs */
        for( int i = 0; i < ledAmount.get(); i++ ) {
            pPixels->setPixelColor(i, pPixels->Color(128 /*red */, 0 /* green */, 0 /* blue */));
        }
        pPixels->show();   // make sure it is visible
        if (SPIFFS.exists ("/homie/config.json") ) 
        { 
          Serial << "Delete Configuration" << endl;
          SPIFFS.remove("/homie/config.json");
        }
        SPIFFS.end();
      } else {
        uint8_t position = (uint8_t) (mButtonPressingCount * 255 / RESET_TRIGGER);
        RainbowCycle(pPixels, &position);
        Serial << mButtonPressingCount << "/" << RESET_TRIGGER << " to reset" << endl;
        mButtonPressingCount++;
      }
    } 
  }
  else 
  {
    if (mButtonPressingCount > 0) {
      /* shutoff the LEDs */
      for( int i = 0; i < ledAmount.get(); i++ ) {
            pPixels->setPixelColor(i, pPixels->Color(0 /*red */, 0 /* green */, 0 /* blue */));
        }
        pPixels->show();   // make sure it is visible
      mButtonPressingCount = 0;
    }
  }
#endif  

}

void log(int level, String message, int statusCode)
{
  String buffer;
  StaticJsonDocument<200> doc;
#ifdef PIR_ENABLE
  // Read the current time
  time_t now; // this is the epoch
  tm tm;      // the structure tm holds time information in a more convient way
  time(&now);
  localtime_r(&now, &tm);
  doc["time"] =  String(String(1900 + tm.tm_year) + "-" + String(tm.tm_mon + 1) + "-" + String(tm.tm_mday) +
              " " + String(tm.tm_hour) + ":" + String(tm.tm_min) + ":" + String(tm.tm_sec));
#endif
  doc["level"] = level;
  doc["message"] = message;
  doc["statusCode"] = statusCode;
  serializeJson(doc, buffer);
  if (mConnected)
  {
    getTopic(LOG_TOPIC, logTopic)

    Homie.getMqttClient().publish(logTopic, 2, false, buffer.c_str());
    delete logTopic;
  }
  Serial << statusCode << "@" << level << " : " << message << endl;
}

#endif