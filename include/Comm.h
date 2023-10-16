#pragma once

#include <Arduino.h>

#include "Battery.h"

class Comm
{
public:
  void loop();
  void setBattery(Battery *battery);
  bool publish_measure_data(const String &type, const String &payload);
  String get_time(void);

private:
  enum state_e
  {
    MODEM_UNINITIALIZED,
    MODEM_POWER_OFF,
    MODEM_POWER_DOWN,
    MODEM_BAT_EN,
    MODEM_RESET,
    MODEM_POWER_UP,
    MODEM_POWER_UP_DELAY,
    MODEM_INIT,
    MODEM_INIT_FAIL_DELAY,
    MODEM_INIT_SIM_UNLOCK,
    MODEM_INIT_WAIT_FOR_NETWORK,
    MODEM_INIT_APN,
    MODEM_INIT_MQTT,
    MODEM_INIT_WAIT_FOR_MQTT,
    MODEM_POWER_ON,
    MODEM_INFINITE_LOOP
  } m_state = MODEM_UNINITIALIZED;

  void state_modem_uninitialized();
  void state_modem_power_off();
  void state_modem_power_down();
  void state_modem_bat_en();
  void state_modem_reset();
  void state_modem_power_up();
  void state_modem_power_up_delay();
  void state_modem_init();
  void state_modem_init_fail_delay();
  void state_modem_init_sim_unlock();
  void state_modem_init_wait_for_network();
  void state_modem_init_apn();
  void state_modem_init_mqtt();
  void state_modem_init_wait_for_mqtt();
  void state_modem_power_on();
  void check_power_condition();
  void publish_init_message();
  String build_topic(String lower_elements);

  void change_state(state_e new_state);

  void mqtt_callback(const String &topic, const String &message);
  void mqtt_service_callback(const String &type, const String &payload);
  void mqtt_irvine_callback(const String &type, const String &payload);

  Battery *m_battery;

  float m_battery_on_voltage = 3.5;
  float m_battery_treshold_voltage = 0.2;

  unsigned long m_sm_timestamp;

  bool requested_power_on = true;

  String m_modem_name;
  String m_modem_info;

  const char *m_pin = "0000";
  const char *m_apn = "internet";
  const char *m_apnUsername = "internet";
  const char *m_apnPassword = "internet";

  const char *m_broker = "iot.2canit.pl";
  const uint16_t m_broker_port = 1883u;
  String m_deviceId = "";

  const char *m_topic_service = "service";

  bool out_connected = false;
  bool out_powered_up = false;
};