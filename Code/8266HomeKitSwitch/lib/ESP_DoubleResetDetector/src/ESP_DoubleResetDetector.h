/****************************************************************************************************************************
   ESP_DoubleResetDetector.h
   For ESP8266 / ESP32 boards

   ESP_DoubleResetDetector is a library for the ESP8266/Arduino platform
   to enable trigger configure mode by resetting ESP32 / ESP8266 twice.

   Forked from DataCute https://github.com/datacute/DoubleResetDetector

   Built by Khoi Hoang https://github.com/khoih-prog/ESP_DoubleResetDetector
   Licensed under MIT license
   Version: 1.1.1

   Version Modified By   Date      Comments
   ------- -----------  ---------- -----------
    1.0.0   K Hoang      15/12/2019 Initial coding
    1.0.1   K Hoang      30/12/2019 Now can use EEPROM or SPIFFS for both ESP8266 and ESP32. RTC still OK for ESP8266
    1.0.2   K Hoang      10/04/2020 Fix bug by left-over cpp file and in example.
    1.0.3   K Hoang      13/05/2020 Update to use LittleFS for ESP8266 core 2.7.1+
    1.1.0   K Hoang      04/12/2020 Add support to LittleFS for ESP32 using LITTLEFS Library
    1.1.1   K Hoang      28/12/2020 Suppress all possible compiler warnings
*****************************************************************************************************************************/

#pragma once

#ifndef ESP_DoubleResetDetector_H
#define ESP_DoubleResetDetector_H

#if defined(ARDUINO) && (ARDUINO >= 100)
  #include <Arduino.h>
#else
  #include <WProgram.h>
#endif

#define ESP_DOUBLE_RESET_DETECTOR_VERSION       "ESP_DoubleResetDetector v1.1.1"
#define ESP_DOUBLERESETDETECTOR_VERSION         ESP_DOUBLE_RESET_DETECTOR_VERSION

//#define ESP_DRD_USE_EEPROM      false
//#define ESP_DRD_USE_LITTLEFS    false
//#define ESP_DRD_USE_SPIFFS      false
//#define ESP8266_DRD_USE_RTC     false   //true

#ifdef ESP32
  #if (!ESP_DRD_USE_EEPROM && !ESP_DRD_USE_SPIFFS && !ESP_DRD_USE_LITTLEFS)
    #warning Neither EEPROM, SPIFFS nor LittleFS selected. Default to EEPROM
    #ifdef ESP_DRD_USE_EEPROM
      #undef ESP_DRD_USE_EEPROM
      #define ESP_DRD_USE_EEPROM      true
    #endif
  #endif
#endif

#ifdef ESP8266
  #if (!ESP8266_DRD_USE_RTC && !ESP_DRD_USE_EEPROM && !ESP_DRD_USE_SPIFFS && !ESP_DRD_USE_LITTLEFS)
    #warning Neither RTC, EEPROM, LITTLEFS nor SPIFFS selected. Default to EEPROM
    #ifdef ESP_DRD_USE_EEPROM
      #undef ESP_DRD_USE_EEPROM
      #define ESP_DRD_USE_EEPROM      true
    #endif
  #endif
#endif

//default to use EEPROM, otherwise, use LITTLEFS (higher priority), then SPIFFS
#if ESP_DRD_USE_EEPROM
  #include <EEPROM.h>

  #define  FLAG_DATA_SIZE     4

  #ifndef EEPROM_SIZE
    #define EEPROM_SIZE     512
  #endif

  #ifndef EEPROM_START
    #define EEPROM_START    256
  #endif

#elif ( ESP_DRD_USE_LITTLEFS || ESP_DRD_USE_SPIFFS )

#include <FS.h>

#ifdef ESP32

  #if ESP_DRD_USE_LITTLEFS
    // The library will be depreciated after being merged to future major Arduino esp32 core release 2.x
    // At that time, just remove this library inclusion
    #include <LITTLEFS.h>             // https://github.com/lorol/LITTLEFS
    #define FileFS   LITTLEFS
  #else
    #include "SPIFFS.h"
    // ESP32 core 1.0.4 still uses SPIFFS
    #define FileFS   SPIFFS
  #endif

#else
  // From ESP8266 core 2.7.1
  #include <LittleFS.h>

  #if ESP_DRD_USE_LITTLEFS
    #define FileFS    LittleFS
  #else
    #define FileFS   SPIFFS
  #endif

#endif    // #if ESP_DRD_USE_EEPROM



#define  DRD_FILENAME     "/drd.dat"

#endif    //#if ESP_DRD_USE_EEPROM

#ifndef DOUBLERESETDETECTOR_DEBUG
#define DOUBLERESETDETECTOR_DEBUG       false
#endif

#define DOUBLERESETDETECTOR_FLAG_SET    0xD0D01234
#define DOUBLERESETDETECTOR_FLAG_CLEAR  0xD0D04321

class DoubleResetDetector
{
  public:
    DoubleResetDetector(int timeout, int address)
    {
#if ESP_DRD_USE_EEPROM
#if (DOUBLERESETDETECTOR_DEBUG)
      Serial.printf("EEPROM size = %d, start = %d\n", EEPROM_SIZE, EEPROM_START);
#endif

      EEPROM.begin(EEPROM_SIZE);
#elif ( ESP_DRD_USE_LITTLEFS || ESP_DRD_USE_SPIFFS )
      // LittleFS / SPIFFS code
      if (!FileFS.begin())
      {
#if (DOUBLERESETDETECTOR_DEBUG)

#if ESP_DRD_USE_LITTLEFS
        Serial.println("LittleFS failed!. Please use SPIFFS or EEPROM.");
#else
        Serial.println("SPIFFS failed!. Please use LittleFS or EEPROM.");
#endif

#endif
      }
#else
#ifdef ESP8266
      //RTC only for ESP8266
#endif
#endif

      this->timeout = timeout * 1000;
      this->address = address;
      doubleResetDetected = false;
      waitingForDoubleReset = false;
    };

    bool detectDoubleReset()
    {
      doubleResetDetected = detectRecentlyResetFlag();

      if (doubleResetDetected)
      {
#if (DOUBLERESETDETECTOR_DEBUG)
        Serial.println("doubleResetDetected");
#endif

        clearRecentlyResetFlag();
      }
      else
      {
#if (DOUBLERESETDETECTOR_DEBUG)
        Serial.println("No doubleResetDetected");
#endif

        setRecentlyResetFlag();
        waitingForDoubleReset = true;
      }

      return doubleResetDetected;

    };

    void loop()
    {
      if (waitingForDoubleReset && millis() > timeout)
      {
#if (DOUBLERESETDETECTOR_DEBUG)
        Serial.println("Stop doubleResetDetecting");
#endif

        stop();
      }
    };

    void stop()
    {
      clearRecentlyResetFlag();
      waitingForDoubleReset = false;
    };

    bool doubleResetDetected;


  private:
    uint32_t DOUBLERESETDETECTOR_FLAG;
    unsigned long timeout;
    int address;
    bool waitingForDoubleReset;

    bool detectRecentlyResetFlag()
    {
#if (ESP_DRD_USE_EEPROM)
      EEPROM.get(EEPROM_START, DOUBLERESETDETECTOR_FLAG);
      doubleResetDetectorFlag = DOUBLERESETDETECTOR_FLAG;

#if (DOUBLERESETDETECTOR_DEBUG)
      Serial.printf("EEPROM Flag read = 0x%X\n", DOUBLERESETDETECTOR_FLAG);
#endif
#elif ( ESP_DRD_USE_LITTLEFS || ESP_DRD_USE_SPIFFS )
      // LittleFS / SPIFFS code
      if (FileFS.exists(DRD_FILENAME))
      {
        // if config file exists, load
        File file = FileFS.open(DRD_FILENAME, "r");

        if (!file)
        {
#if (DOUBLERESETDETECTOR_DEBUG)
          Serial.println("Loading config file failed");
#endif
        }

        file.readBytes((char *) &DOUBLERESETDETECTOR_FLAG, sizeof(DOUBLERESETDETECTOR_FLAG));
        doubleResetDetectorFlag = DOUBLERESETDETECTOR_FLAG;

#if (DOUBLERESETDETECTOR_DEBUG)

#if ESP_DRD_USE_LITTLEFS
        Serial.printf("LittleFS Flag read = 0x%X\n", DOUBLERESETDETECTOR_FLAG);
#else
        Serial.printf("SPIFFS Flag read = 0x%X\n", DOUBLERESETDETECTOR_FLAG);
#endif

#endif

        file.close();
      }
#else
#ifdef ESP8266
      //RTC only for ESP8266
      ESP.rtcUserMemoryRead(address, &doubleResetDetectorFlag, sizeof(doubleResetDetectorFlag));
#endif
#endif

      doubleResetDetected = (doubleResetDetectorFlag == DOUBLERESETDETECTOR_FLAG_SET);
      return doubleResetDetected;
    };

    void setRecentlyResetFlag()
    {
      doubleResetDetectorFlag = DOUBLERESETDETECTOR_FLAG_SET;

      DOUBLERESETDETECTOR_FLAG = DOUBLERESETDETECTOR_FLAG_SET;

#if (ESP_DRD_USE_EEPROM)
      EEPROM.put(EEPROM_START, DOUBLERESETDETECTOR_FLAG);
      EEPROM.commit();

#if (DOUBLERESETDETECTOR_DEBUG)
      delay(1000);
      EEPROM.get(EEPROM_START, DOUBLERESETDETECTOR_FLAG);

      Serial.printf("SetFlag write = 0x%X\n", DOUBLERESETDETECTOR_FLAG);
#endif
#elif ( ESP_DRD_USE_LITTLEFS || ESP_DRD_USE_SPIFFS )
      // LittleFS / SPIFFS code
      File file = FileFS.open(DRD_FILENAME, "w");
#if (DOUBLERESETDETECTOR_DEBUG)
      Serial.println("Saving config file...");
#endif

      if (file)
      {
        file.write((uint8_t *) &DOUBLERESETDETECTOR_FLAG, sizeof(DOUBLERESETDETECTOR_FLAG));
        file.close();
#if (DOUBLERESETDETECTOR_DEBUG)
        Serial.println("Saving config file OK");
#endif
      }
      else
      {
#if (DOUBLERESETDETECTOR_DEBUG)
        Serial.println("Saving config file failed");
#endif
      }
#else
#ifdef ESP8266
      //RTC only for ESP8266
      ESP.rtcUserMemoryWrite(address, &doubleResetDetectorFlag, sizeof(doubleResetDetectorFlag));
#endif
#endif
    };


    void clearRecentlyResetFlag()
    {
      doubleResetDetectorFlag = DOUBLERESETDETECTOR_FLAG_CLEAR;
      DOUBLERESETDETECTOR_FLAG = DOUBLERESETDETECTOR_FLAG_CLEAR;

#if (ESP_DRD_USE_EEPROM)
      //DOUBLERESETDETECTOR_FLAG = DOUBLERESETDETECTOR_FLAG_CLEAR;
      EEPROM.put(EEPROM_START, DOUBLERESETDETECTOR_FLAG);
      EEPROM.commit();

#if (DOUBLERESETDETECTOR_DEBUG)
      delay(1000);
      EEPROM.get(EEPROM_START, DOUBLERESETDETECTOR_FLAG);

      Serial.printf("ClearFlag write = 0x%X\n", DOUBLERESETDETECTOR_FLAG);
#endif
#elif ( ESP_DRD_USE_LITTLEFS || ESP_DRD_USE_SPIFFS )
      // LittleFS / SPIFFS code
      File file = FileFS.open(DRD_FILENAME, "w");
#if (DOUBLERESETDETECTOR_DEBUG)
      Serial.println("Saving config file...");
#endif

      if (file)
      {
        file.write((uint8_t *) &DOUBLERESETDETECTOR_FLAG, sizeof(DOUBLERESETDETECTOR_FLAG));
        file.close();
#if (DOUBLERESETDETECTOR_DEBUG)
        Serial.println("Saving config file OK");
#endif
      }
      else
      {
#if (DOUBLERESETDETECTOR_DEBUG)
        Serial.println("Saving config file failed");
#endif
      }

#else
#ifdef ESP8266
      //RTC only for ESP8266
      ESP.rtcUserMemoryWrite(address, &doubleResetDetectorFlag, sizeof(doubleResetDetectorFlag));
#endif
#endif
    };

    uint32_t doubleResetDetectorFlag;
};
#endif // ESP_DoubleResetDetector_H
