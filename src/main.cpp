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

HomieNode oneLedNode /* to rule them all */("led", "led", "color");
HomieNode lampNode("lamp", "lamp", "switch");
HomieNode dimmNode("dimm", "dimm", "dimmer");
#ifdef PIR_ENABLE
HomieNode monitor("monitor", "Monitor motion", "contact");
#endif
#ifdef TEMP_ENABLE
HomieNode temperatureNode("temperature", "Temparture", "number");
#endif

bool mHomieConfigured = false;
unsigned long mLastLedChanges = 0U;
int mSentPwmValue = 0;
bool somethingReceived = false;

unsigned int mButtonPressingCount = 0;        /**< Delay before everything is reset */
unsigned int mColorFadingCount = FADE_MAXVALUE;
bool mLastMotion=false;
int mButtonLastState=false;
unsigned long mShutoffAfterMotion = TIME_UNDEFINED;     /**< Time, when LED has to be deactivated after motion */
bool mConnected = false;
    
uint8_t mRainbowPosition = 1; /**< Index, used for animated rainbow (<code>0</code> deactivates it)*/

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

#ifndef NOBUTTON
  /* Send status information at start, based on Button input */
  if (digitalRead(GPIO_BUTTON) == HIGH) {
    lampNode.setProperty("value").send(HOMIE_TRUE);
    dimmNode.setProperty("value").send(String(PWM_MAXVALUE));
  } else {
    lampNode.setProperty("value").send(HOMIE_FALSE);
    dimmNode.setProperty("value").send(String("0"));
  }
#endif
  mConnected = true;
  default:
    break;
  }
}

int setRgbColor(tm* tm) {  
  #ifdef PIR_ENABLE    
      uint32_t color = extractColor(dayColor.get(), strlen(dayColor.get()) );
      int maxPercent = (dayPercent.get() * PWM_MAXVALUE) / HOMIE_MAXPERCENT;

      if ((tm != NULL) &&
          ((nightStartHour.get() <= tm->tm_hour) || (tm->tm_hour <= nightEndHour.get()) ) 
          ) {
          color = extractColor(nightColor.get(), strlen(nightColor.get()) );
          maxPercent = (nightPercent.get() * PWM_MAXVALUE) / HOMIE_MAXPERCENT;
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
        for( int i = 0; i < ledAmount.get(); i++ ) {
          pPixels->setBrightness(mColorFadingCount);
          pPixels->setPixelColor(i, color);
        }
      }

      /* Handle PWM led */
      if (maxPercent > 0) {
        led.setPercent(maxPercent);
      } else {
        /* At night, deactivate the white LED */
        ;
      }
      return maxPercent;
      #else
      return 0;
      #endif
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
    monitor.setProperty("motion").send(String(mLastMotion ? HOMIE_TRUE : HOMIE_FALSE));
    
    if (mLastMotion == HIGH) {
      
      /* clear LEDs */
      if (!somethingReceived) {
        pPixels->clear();
        pPixels->show();
      }
      somethingReceived = true; // Stop the animation, as a montion was detected */

      int maxPercent = setRgbColor(&tm);
      led.setPercent(maxPercent);

      log(LEVEL_DEBUG, String("") +
          String("Fade:" + String(maxPercent) + "%" +
          String("RGB :" + String(mColorFadingCount) + "%") +
          " Time: " + millis() + " left: " + String((mShutoffAfterMotion- millis())/1000) + String("s")), STATUS_MOTION_DETECTED);
      

      mShutoffAfterMotion = millis() + (minimumActivation.get() * 1000);
      log(LEVEL_DEBUG,String("Update " + String(mShutoffAfterMotion) ), STATUS_PWM_RETRIGGER);
    }
  }
#endif

#ifdef TEMP_ENABLE
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
#endif

  // Feed the dog -> ESP stay alive
  ESP.wdtFeed();
}

bool switchHandler(const HomieRange& range, const String& value) {
  if (range.isRange) return false;  // only one switch is present
  if (value == "off" || value == "Off" || value == "OFF" || value == "false") {
    led.setOff();
    dimmNode.setProperty("value").send(value);
    lampNode.setProperty("value").send(value);
  } else if (value == "on" || value == "On" || value == "ON" || value == "true") {
    led.setOn();
    dimmNode.setProperty("value").send(value);
    lampNode.setProperty("value").send(value);
  } else if ( value.length() > 0 && isDigit(value.charAt(0))  ) {
      int targetVal = value.toInt();
      if ((targetVal >= 0) && (targetVal <= PWM_MAXVALUE)) {
        log(LEVEL_LOG, String("MQTT | Dimm to ") + String(value.toInt()) + String( "% (0..1023)"), STATUS_PWM_STARTS);
        led.setPercent(targetVal);
        dimmNode.setProperty("value").send(value);
        if (targetVal == 0) {
          lampNode.setProperty("value").send(HOMIE_FALSE);
        } else {
          lampNode.setProperty("value").send(HOMIE_TRUE);
        }
      } else {
          log(LEVEL_ERROR, String("MQTT | Unknown percent: '") + String(value) + String( "'"), STATUS_PWM_STARTS);
      }
  } else {
    log(LEVEL_ERROR, String(value), STATUS_UNKNOWN_CMD);
  }
  somethingReceived = true; // Stop animation
  log(LEVEL_DEBUG, "Received switch command", STATUS_MQTT_DETECTED);
  return true;
}

bool allLedsHandler(const HomieRange& range, const String& value) {
  if (range.isRange) return false;  // only one switch is present

  somethingReceived = true; // Stop animation
  log(LEVEL_DEBUG, "Received rgb command", STATUS_MQTT_DETECTED);

  int sep1 = value.indexOf(',');
  int sep2 = value.indexOf(',', sep1 + 1);
  int red = value.substring(0,sep1).toInt();
  int green = value.substring(sep1 + 1, sep2).toInt();
  int blue = value.substring(sep2 + 1, value.length()).toInt();

  uint8_t r = (red * 255) / 250;
  uint8_t g = (green *255) / 250;
  uint8_t b = (blue *255) / 250;


  if (pPixels) {
    uint32_t c = pPixels->Color(r,g,b);
    pPixels->setBrightness(255);
    pPixels->fill(c);
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
  mShutoffAfterMotion = TIME_FADE_DONE;

  log(LEVEL_DEBUG, "Received light command", STATUS_MQTT_DETECTED);

  if (pPixels != NULL) {
    pPixels->clear();  // Initialize all pixels to 'off'
    if (value == "off" || value == "Off" || value == "OFF") {
        pPixels->setPixelColor(range.index - 1, pPixels->ColorHSV(0, 0, 0));
    } else {
      /* Parse the color */
      pPixels->setPixelColor(range.index - 1, extractColor(value.c_str(), value.length()));
    }
    pPixels->show();   // make sure it is visible
  }

  if (value == "off" || value == "Off" || value == "OFF") {
    led.setOff();
  } else {
    led.setOn();
  }

  Homie.getLogger() << "Led " << range.index << " is " << value << endl;

  return true;
}

void setup() {
  /* Load Filesystem */
  SPIFFS.begin();
  Serial.begin(115200);
  Serial << endl << endl;
  Homie_setFirmware(FIRMWARE_NAME, FIRMWARE_VERSION);
  Homie.setLoopFunction(loopHandler);
  Homie.onEvent(onHomieEvent);

  // Load the settings and  set default values
  ledAmount.setDefaultValue(NUMBER_LEDS).setValidator([] (long candidate) {
    return (candidate > 0) && (candidate < 2048);
  });

  #ifdef PIR_ENABLE
  monitor.advertise("motion").setName("Monitor motion").setDatatype("boolean");
  #endif
  oneLedNode.advertise("ambient").setName("All Leds")
                            .setDatatype("color").setFormat("rgb")
                            .settable(allLedsHandler);
  lampNode.advertise("value").setName("Value")
                                      .setDatatype("boolean")
                                      .settable(switchHandler);
  dimmNode.advertise("value").setName("Dimmer")
                                      .setDatatype("integer")
                                      .setUnit("%")
                                      .setFormat("0:" PWM_HOMIEMAXVALUE)
                                      .settable(switchHandler);
#ifdef TEMP_ENABLE
  temperatureNode.advertise(NODE_TEMPERATUR).setName("Degrees")
                                      .setDatatype("float")
                                      .setUnit("ºC");                                      
#endif

  #ifdef PIR_ENABLE
  dayColor.setDefaultValue("off").setValidator([] (const char *candidate) {
    return extractColor(candidate, strlen(candidate)) != 0xFFFFFFFF;
  });
  nightColor.setDefaultValue("red").setValidator([] (const char *candidate) {
    return extractColor(candidate, strlen(candidate)) != 0xFFFFFFFF;
  });
  dayPercent.setDefaultValue(90).setValidator([] (long candidate) {
    return (candidate >= 0) && (candidate <= HOMIE_MAXPERCENT);
  });
  nightPercent.setDefaultValue(10).setValidator([] (long candidate) {
    return (candidate >= 0) && (candidate <= HOMIE_MAXPERCENT);
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
#ifdef TEMP_ENABLE
  oneWireSensorAvail.setDefaultValue(false).setValidator([] (int candidate) {
    return true;
  });
#endif

  Homie.setup();
  mHomieConfigured = Homie.isConfigured();

  pPixels = new Adafruit_NeoPixel(ledAmount.get(), GPIO_WS2812, NEO_GRB + NEO_KHZ800);

  pPixels->begin();
  pPixels->clear();
  Serial << "WS2812 Strip initialized with " << (ledAmount.get()) << " leds" << endl;
#ifndef NOBUTTON
  pinMode(GPIO_BUTTON, INPUT); // GPIO0 as input
#endif 
#ifdef PIR_ENABLE
  pinMode(GPIO_PIR, INPUT);

  // always shutdown LED after controller was started
  mShutoffAfterMotion = millis() + (minimumActivation.get() * 1000);
#endif
#ifdef TEMP_ENABLE 
  if (oneWireSensorAvail.get()) {
    sensors.begin();
    for(int j=0; j < TEMP_SENSOR_MEASURE_SERIES && sensors.getDeviceCount() == 0; j++) {
        delay(100);
        sensors.begin();
        Serial << "Reset 1-Wire Bus" << endl;
      }
  }
#endif
/* Always activate all LEDs if not controllable */
#ifdef NOBUTTON
  led.setPercent(PWM_MAXVALUE);
  Serial << "PWM  LED dimming to " << (led.getPercent()) << " % (0 .. " << PWM_MAXVALUE << ")" << endl;
#endif
}

void loop() {
  static int oddCalled = 0;
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
    if ( ((millis() - mLastLedChanges) >= FADE_INTERVAL) ||
        (mLastLedChanges == 0U) ) {
      /* call rainbow only, when activated */
      if (mRainbowPosition > 0) {
        RainbowCycle(pPixels, &mRainbowPosition);
      }
      /* Handle overflow */
      if (mRainbowPosition < 254)
      {
        mRainbowPosition++;
      } else {
        mRainbowPosition = 1;
      }

      if (millis() > mShutoffAfterMotion) {
        log(LEVEL_DEBUG,String("Initial finished to ") + String(mShutoffAfterMotion) + String("s"), STATUS_PWM_FINISHED);
        led.setPercent(0);
        pPixels->setBrightness(0);
        pPixels->show();
        mShutoffAfterMotion = TIME_FADE_DONE;
      }
      mLastLedChanges = millis();
    }
  /* the chip has to do something with color */
  } else {
    /* Execute this at the first time to deactivate the rainbow animation */
    if (mRainbowPosition != 0) {
      mRainbowPosition = 0;
      setRgbColor(NULL);
    }

#ifdef PIR_ENABLE
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
                          String(led.getCurrentPwm()) + String(" targets ") +
                          String(led.getPercent()) + String("% 0..1023")
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

      oddCalled = (oddCalled + 1) % 10;
      int pwmValue = led.getCurrentPwm();
      if ((oddCalled == 0) && (mConnected) && /* Update MQTT only every tenth call */
          (mSentPwmValue != pwmValue)) { 
        mSentPwmValue = pwmValue;
        dimmNode.setProperty("value").send(String(pwmValue));
      }

      mLastLedChanges = millis();
    }
#endif
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
  int currentButton = digitalRead(GPIO_BUTTON) ;
  // Handle Button to toggle white LEDs
    if (mHomieConfigured) {
      if (currentButton != mButtonLastState) {
        log(LEVEL_DEBUG, "HW Button", STATUS_HARDWARE_BUTTON);
        somethingReceived = true; // Stop animation
        /* deactivate rgb LEDs */
        pPixels->fill(pPixels->Color(0,0,0));
        pPixels->setBrightness(0);
        pPixels->show();

        /* Directly map input switch to PWM output */
        if (currentButton == HIGH ) {
          led.setOn();
        }
        if (currentButton == LOW ) {
          led.setOff();
        }
      }
      mButtonLastState = currentButton;
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
