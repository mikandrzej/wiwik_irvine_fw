#pragma once

#include <Arduino.h>

#include "Battery.h"
#include <ArduinoJson.h>

class Comm
{
public:
  void loop();
  void setBattery(Battery *battery);
  bool publish_measure_data(const String &type, const String &sensor, const float value);
  bool publish_measure_data(const String &type, const String &sensor, DynamicJsonDocument &jsonDocument);
  void publish_debug_data(const String &message);
  String get_time(void);
  String getDeviceId();

// private:
  enum state_e
  {
    MODEM_UNINITIALIZED,
    MODEM_POWER_OFF,
    MODEM_POWER_DOWN,
    MODEM_RESET,
    MODEM_POWER_UP,
    MODEM_POWER_UP_DELAY,
    MODEM_INIT,
    MODEM_INIT_FAIL_DELAY,
    MODEM_INIT_SIM_UNLOCK,
    MODEM_INIT_WAIT_FOR_NETWORK,
    MODEM_INIT_APN,
    MODEM_INIT_GPS,
    MODEM_INIT_MQTT,
    MODEM_INIT_WAIT_FOR_MQTT,
    MODEM_POWER_ON,
    MODEM_INFINITE_LOOP
  } m_state = MODEM_UNINITIALIZED;

  void state_modem_uninitialized();
  void state_modem_power_off();
  void state_modem_power_down();
  void state_modem_reset();
  void state_modem_power_up();
  void state_modem_power_up_delay();
  void state_modem_init();
  void state_modem_init_fail_delay();
  void state_modem_init_sim_unlock();
  void state_modem_init_wait_for_network();
  void state_modem_init_apn();
  void state_modem_init_gps();
  void state_modem_init_mqtt();
  void state_modem_init_wait_for_mqtt();
  void state_modem_power_on();
  void state_modem_loop();
  void check_power_condition();
  void publish_init_message();
  String build_topic(String lower_elements);
  String build_measure_topic(const String &sensor_type, const String &sensor_address);

  void change_state(state_e new_state);

  void mqtt_callback(const String &topic, const String &message);
  void mqtt_service_callback(const String &type, const String &payload);
  void mqtt_irvine_callback(const String &type, const String &payload);

  Battery *m_battery;

  float m_battery_on_voltage = 3.0;
  float m_battery_treshold_voltage = 0.2;

  unsigned long m_sm_timestamp;

  bool requested_power_on = true;

  String m_modem_name;
  String m_modem_info;
  String m_sim_ccid;
  String m_imsi;

  const char *m_pin = "4967";
  const char *m_apn = "internet";
  const char *m_apnUsername = "internet";
  const char *m_apnPassword = "internet";

  bool out_connected = false;
  bool out_powered_up = false;

  uint32_t m_lastNetworkShot = 0u;
};

extern Comm comm;