#pragma once

#include <Arduino.h>

#include "Battery.h"

class Comm {
public:
  bool init();
  void cyclic();
  void setBattery(Battery *battery);

private:
  enum { UNINITIALIZED, POWER_OFF, POWERING_ON, POWER_ON, POWERING_OFF } state;

  void state_power_off();
  void state_powering_on();
  void state_power_on();
  void state_powering_off();


  Battery *m_battery;
};