#include <Arduino.h>
#include <arduino_homekit_server.h>
#include <WiFiManager.h>
#include "config.h"
#include <ESP8266WiFi.h>
#include <ESP_DoubleResetDetector.h> 

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

// access your HomeKit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_switch_on;

#define PIN_SWITCH                  4

#define DRD_TIMEOUT                 5               //两次Rst最大间隔
#define DRD_ADDRESS                 0

DoubleResetDetector* m_doubleResetDetector = nullptr;

void cha_switch_on_setter(const homekit_value_t value) {
	bool on = value.bool_value;
	cha_switch_on.value.bool_value = on;	
	digitalWrite(PIN_SWITCH, on ? HIGH : LOW);
  LOG_D("Switch: %s", on ? "ON" : "OFF");
}

char macStr[19] = { 0 };

void my_homekit_setup() {
	pinMode(PIN_SWITCH, OUTPUT);
	digitalWrite(PIN_SWITCH, LOW);

	cha_switch_on.setter = cha_switch_on_setter;

	uint8_t mac[6];
  WiFi.macAddress(mac);
  sprintf(macStr, "SWITCH%02X%02X%02X%02X%02X%02X", mac[ 0], mac[ 1], mac[ 2], mac[ 3], mac[ 4], mac[ 5]);
  config.accessories[0]->services[0]->characteristics[0]->value.string_value = macStr;//增加MAC地址，做为设备唯一名字
	
  arduino_homekit_setup(&config);
}

void my_homekit_loop() {
	arduino_homekit_loop();
}

void setup() {
	Serial.begin(115200);
  WiFiManager wm;

  m_doubleResetDetector = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
  
  if (m_doubleResetDetector->detectDoubleReset()) 
  {
    Serial.printf("------Detect Double Reset Viable.\n");
    homekit_storage_reset();
    wm.resetSettings();
    ESP.reset();
    return;
  }else
  {
    Serial.printf("------Detect Double Reset Passed.\n");
  }

  WiFi.mode(WIFI_STA);
  // wm.resetSettings();
  bool res = wm.autoConnect(AP_SSID,AP_PASSWORD);
  if(!res) {
        while (1)
        {
          delay(500);
          Serial.println("Failed to connect");
        }
  } 
  else {
      Serial.println("connected...yeey :)");
  }
	//homekit_storage_reset(); // to remove the previous HomeKit pairing storage when you first run this new HomeKit example
	my_homekit_setup();
}

void loop() {
  m_doubleResetDetector->loop();
	my_homekit_loop();
	delay(10);
}