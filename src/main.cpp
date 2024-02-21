#include <Arduino.h>

#include <WiFi.h>

#include "Irvine.h"
#include "Configuration.h"
#include "ESP32TimerInterrupt.h"

#include <Update.h>
#define TIMER0_INTERVAL_MS        1000

Irvine irvine;

uint32_t software_version = 5u;

void setup()
{
  Serial.begin(115200);

  pinMode(21, OUTPUT); // 1wire
  pinMode(22, OUTPUT);  

  configuration.initSource();
  configuration.readConfig();

  Serial.printf("Software version: %u\r\n", software_version);
}

void loop()
{
  irvine.loop();
  configuration.cyclic();
}
