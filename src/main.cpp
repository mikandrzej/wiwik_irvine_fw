#include <Arduino.h>

#include <WiFi.h>

#include "Irvine.h"

Irvine irvine;

void setup() {
    Serial.begin(115200);

    String deviceMac = WiFi.macAddress();
    deviceMac.replace(":", "");
    String deviceId = "irvine_" + deviceMac;

    irvine.setDeviceId(deviceId);
  // put your setup code here, to run once:
}

void loop() {
  irvine.loop();
  // put your main code here, to run repeatedly:
}
