#include <Arduino.h>
#include <arduino_homekit_server.h>
#include <WiFiManager.h>
#include "config.h"

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

// access your HomeKit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_switch_on;

#define PIN_SWITCH 2

void cha_switch_on_setter(const homekit_value_t value) {
	bool on = value.bool_value;
	cha_switch_on.value.bool_value = on;	
	digitalWrite(PIN_SWITCH, on ? HIGH : LOW);
  LOG_D("Switch: %s", on ? "ON" : "OFF");
}

void my_homekit_setup() {
	pinMode(PIN_SWITCH, OUTPUT);
	digitalWrite(PIN_SWITCH, LOW);

	cha_switch_on.setter = cha_switch_on_setter;
	arduino_homekit_setup(&config);
}

void my_homekit_loop() {
	arduino_homekit_loop();
}

void setup() {
	Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
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
	my_homekit_loop();
	delay(10);
}