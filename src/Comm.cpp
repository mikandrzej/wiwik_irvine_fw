#include "Battery.h"
#include "Comm.h"

bool Comm::init() {
  if (NULL == m_battery)
    return false;
  state = POWER_OFF;
  return true;
}

void Comm::cyclic() {
  switch (state) {
  case POWER_OFF:
    state_power_off();
    break;
  case POWERING_ON:
    state_powering_on();
    break;
  case POWER_ON:
    state_power_on();
    break;
  case POWERING_OFF:
    state_powering_off();
    break;
  }
}
void Comm::setBattery(Battery *battery) { m_battery = battery; }

void Comm::state_power_off() {

}
void Comm::state_powering_on() {}
void Comm::state_power_on() {}
void Comm::state_powering_off() {}