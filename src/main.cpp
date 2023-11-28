#include <Arduino.h>

#include <WiFi.h>

#include "Irvine.h"
#include "Configuration.h"

Irvine irvine;

void setup()
{
  Serial.begin(115200);

  pinMode(21, OUTPUT); // led
  pinMode(22, INPUT);  // charger detection

  // configuration.initSource();
  // configuration.readConfig();
}

void loop()
{
  irvine.loop();
}
