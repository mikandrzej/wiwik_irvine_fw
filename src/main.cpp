#include <Arduino.h>

#include "Irvine.h"

Irvine irvine;


void setup() {
    Serial.begin(115200);
  // put your setup code here, to run once:
}

void loop() {
  irvine.loop();
  // put your main code here, to run repeatedly:
}
