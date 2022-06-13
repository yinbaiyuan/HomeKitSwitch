/****************************************************************************************************************************
   minimal.ino
   For ESP8266 / ESP32 boards

   ESP_DoubleResetDetector is a library for the ESP8266/Arduino platform
   to enable trigger configure mode by resetting ESP32 / ESP8266 twice.

   Based on and modified from DataCute https://github.com/datacute/DoubleResetDetector

   Built by Khoi Hoang https://github.com/khoih-prog/ESP_DoubleResetDetector
   Licensed under MIT license
   Version: 1.1.0

   Version Modified By   Date      Comments
   ------- -----------  ---------- -----------
    1.0.0   K Hoang      15/12/2019 Initial coding
    1.0.1   K Hoang      30/12/2019 Now can use EEPROM or SPIFFS for both ESP8266 and ESP32. RTC still OK for ESP8266
    1.0.2   K Hoang      10/04/2020 Fix bug by left-over cpp file and in example.
    1.0.3   K Hoang      13/05/2020 Update to use LittleFS for ESP8266 core 2.7.1+
    1.1.0   K Hoang      04/12/2020 Add support to LittleFS for ESP32 using LITTLEFS Library
 *****************************************************************************************************************************/

// These defines must be put before #include <ESP_DoubleResetDetector.h>
// to select where to store DoubleResetDetector's variable.
// For ESP32, You must select one to be true (EEPROM or SPIFFS)
// For ESP8266, You must select one to be true (RTC, EEPROM, LITTLEFS or SPIFFS)
// Otherwise, library will use default EEPROM storage


#ifdef ESP8266
  #define ESP8266_DRD_USE_RTC     false   //true
#endif

#define ESP_DRD_USE_LITTLEFS    true
#define ESP_DRD_USE_SPIFFS      false
#define ESP_DRD_USE_EEPROM      false

#define DOUBLERESETDETECTOR_DEBUG       true  //false

#include <ESP_DoubleResetDetector.h>      //https://github.com/khoih-prog/ESP_DoubleResetDetector

// Number of seconds after reset during which a 
// subseqent reset will be considered a double reset.
#define DRD_TIMEOUT 10

// RTC Memory Address for the DoubleResetDetector to use
#define DRD_ADDRESS 0

DoubleResetDetector* drd;

#ifdef ESP32

  // For ESP32
  #ifndef LED_BUILTIN
    #define LED_BUILTIN       2         // Pin D2 mapped to pin GPIO2/ADC12 of ESP32, control on-board LED
  #endif

  #define LED_OFF     LOW
  #define LED_ON      HIGH

#else

  // For ESP8266
  #define LED_ON      LOW
  #define LED_OFF     HIGH

#endif

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(115200);
  Serial.println("\nStarting minimal example for ESP_DoubleResetDetector");
  
  drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);

  if (drd->detectDoubleReset()) 
  {
    Serial.println("Double Reset Detected");
    digitalWrite(LED_BUILTIN, LED_ON);
  } 
  else 
  {
    Serial.println("No Double Reset Detected");
    digitalWrite(LED_BUILTIN, LED_OFF);
  }

}

void loop()
{
  // Call the double reset detector loop method every so often,
  // so that it can recognise when the timeout expires.
  // You can also call drd.stop() when you wish to no longer
  // consider the next reset as a double reset.
  drd->loop();
}
