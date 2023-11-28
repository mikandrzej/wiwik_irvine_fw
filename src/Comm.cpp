#include "Battery.h"
#include "Comm.h"
#include <WiFi.h>

#include "EgTinyGsm.h"

#define TINY_GSM_MODEM_SIM7600 // A7670's AT instruction is compatible with SIM7600
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <TinyGPSPlus.h>

#define SerialAT Serial1
#define UART_BAUD 115200
#define PIN_TX 26
#define PIN_RX 27

#define PWR_PIN 4
#define RESET 5
#define BAT_EN 12

EgTinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

TinyGPSPlus gps;

void Comm::loop()
{
  check_power_condition();

  switch (m_state)
  {
  case MODEM_UNINITIALIZED:
    state_modem_uninitialized();
    break;
  case MODEM_POWER_OFF:
    state_modem_power_off();
    break;
  case MODEM_POWER_DOWN:
    state_modem_power_down();
    break;
  case MODEM_BAT_EN:
    state_modem_bat_en();
    break;
  case MODEM_RESET:
    state_modem_reset();
    break;
  case MODEM_POWER_UP:
    state_modem_power_up();
    break;
  case MODEM_POWER_UP_DELAY:
    state_modem_power_up_delay();
    break;
  case MODEM_INIT:
    state_modem_init();
    break;
  case MODEM_INIT_FAIL_DELAY:
    state_modem_init_fail_delay();
    break;
  case MODEM_INIT_SIM_UNLOCK:
    state_modem_init_sim_unlock();
    break;
  case MODEM_INIT_WAIT_FOR_NETWORK:
    state_modem_init_wait_for_network();
    break;
  case MODEM_INIT_APN:
    state_modem_init_apn();
    break;
  case MODEM_INIT_GPS:
    state_modem_init_gps();
    break;
  case MODEM_INIT_MQTT:
    state_modem_init_mqtt();
    break;
  case MODEM_INIT_WAIT_FOR_MQTT:
    state_modem_init_wait_for_mqtt();
    break;
  case MODEM_POWER_ON:
    state_modem_power_on();
    break;
  case MODEM_INFINITE_LOOP:
    break;
  }
}
void Comm::setBattery(Battery *battery) { m_battery = battery; }

void Comm::state_modem_uninitialized()
{
  if (NULL == m_battery)
    return;

  SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

  pinMode(BAT_EN, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(PWR_PIN, OUTPUT);

  digitalWrite(BAT_EN, LOW);
  digitalWrite(RESET, LOW);
  digitalWrite(PWR_PIN, HIGH);

  String deviceMac = WiFi.macAddress();
  deviceMac.replace(":", "");
  m_deviceId = "irvine_" + deviceMac;
  Serial.println("Deviced id: " + m_deviceId);

  Serial.println("Modem power off");

  change_state(MODEM_POWER_OFF);
}

void Comm::state_modem_power_off()
{
  if (true == requested_power_on)
  {
    if (m_battery->getBatteryVoltage() > (m_battery_on_voltage + m_battery_treshold_voltage))
    {
      change_state(MODEM_POWER_DOWN);

      Serial.println("Modem power down");
    }
  }
}

void Comm::state_modem_power_down()
{
  digitalWrite(BAT_EN, LOW);
  digitalWrite(RESET, HIGH);
  digitalWrite(PWR_PIN, HIGH);

  change_state(MODEM_BAT_EN);

  Serial.println("Modem bat enable");
}

void Comm::state_modem_bat_en()
{
  if (millis() - m_sm_timestamp > 1000)
  {
    /* Enable modem power supply */
    digitalWrite(BAT_EN, HIGH);

    change_state(MODEM_RESET);
    Serial.println("Modem reset");
  }
}

void Comm::state_modem_reset()
{
  if (millis() - m_sm_timestamp > 3000)
  {
    /* Reset A76xx */
    digitalWrite(RESET, LOW);

    change_state(MODEM_POWER_UP);
    Serial.println("Modem power up");
  }
}

void Comm::state_modem_power_up()
{
  if (millis() - m_sm_timestamp > 300)
  {
    /* Power up A76xx */
    digitalWrite(PWR_PIN, LOW);

    change_state(MODEM_POWER_UP_DELAY);
    Serial.println("Modem power delay");
  }
}

void Comm::state_modem_power_up_delay()
{
  if (millis() - m_sm_timestamp > 10000)
  {
    change_state(MODEM_INIT);
    Serial.println("Modem init");
  }
}

void Comm::state_modem_init()
{
  if (modem.init())
  {
    m_modem_name = modem.getModemName();
    Serial.println("Modem name: " + m_modem_name);
    m_modem_info = modem.getModemInfo();
    Serial.println("Modem info: " + m_modem_info);

    change_state(MODEM_INIT_SIM_UNLOCK);
  }
  else
  {
    Serial.println("Failed init failed, delay 10s");

    change_state(MODEM_INIT_FAIL_DELAY);
  }
}

void Comm::state_modem_init_fail_delay()
{
  if (millis() - m_sm_timestamp > 10000)
  {
    change_state(MODEM_POWER_OFF);
  }
}

void Comm::state_modem_init_sim_unlock()
{
  SimStatus sim_status = modem.getSimStatus(0);
  if (modem.getSimStatus() == SIM_LOCKED)
  {
    if (modem.simUnlock(m_pin))
    {
      Serial.println("Waiting for network");

      change_state(MODEM_INIT_WAIT_FOR_NETWORK);
    }
    else
    {
      Serial.println("Unable to unlock SIM card");
      change_state(MODEM_INFINITE_LOOP);
    }
  }
  else
  {
    Serial.println("Wrong SIM satus: " + String(sim_status));
    change_state(MODEM_INFINITE_LOOP);
  }
}

void Comm::state_modem_init_wait_for_network()
{
  if (modem.isNetworkConnected())
  {
    Serial.println("Network connected - init APN");

    m_sim_ccid = modem.getSimCCID();
    Serial.println("SIM CCID: " + m_sim_ccid);
    m_imsi = modem.getIMSI();
    Serial.println("IMSI: " + m_imsi);

    change_state(MODEM_INIT_APN);
  }
  else if (millis() - m_sm_timestamp > 60000)
  {
    Serial.println("Network not found - reset");
    change_state(MODEM_POWER_DOWN);
  }
}

void Comm::state_modem_init_apn()
{
  if (modem.gprsConnect(m_apn, m_apnUsername, m_apnPassword))
  {
    if (modem.isGprsConnected())
    {
      Serial.println("GPRS Connected");
      change_state(MODEM_INIT_GPS);
    }
  }
}

void Comm::state_modem_init_gps()
{
  bool result = false;

  do
  {
    modem.sendAT("+CGNSSPWR=0");
    if (modem.waitResponse(30000) != 1)
    {
      break;
    }
    modem.sendAT("+CGNSSPWR=1");
    if (modem.waitResponse(30000) != 1)
    {
      break;
    }
    if (modem.waitResponse(10000UL, "+CGNSSPWR: READY!") != 1)
    {
      break;
    }
    modem.sendAT("+CGNSSMODE=1");
    if (modem.waitResponse(30000) != 1)
    {
      break;
    }
    result = true;
  } while (false);

  if (result)
  {
    Serial.println("GPS lauched");
    change_state(MODEM_INIT_MQTT);
  }
}

void Comm::state_modem_init_mqtt()
{
  mqtt.setServer(m_broker, m_broker_port);
  mqtt.setCallback(
      [this](char *topic, uint8_t *payload, unsigned int len)
      {
        mqtt_callback(String(topic), String((const char *)payload, len));
      });

  Serial.println("Waiting for MQTT");

  change_state(MODEM_INIT_WAIT_FOR_MQTT);
}

void Comm::state_modem_init_wait_for_mqtt()
{
  if (mqtt.connect(m_deviceId.c_str()))
  {
    Serial.println("MQTT Connected - sending hello message");

    mqtt.subscribe(build_topic(m_topic_service).c_str());

    publish_init_message();

    change_state(MODEM_POWER_ON);
  }
}

void Comm::state_modem_power_on()
{
  if (!modem.isNetworkConnected())
  {
    change_state(MODEM_INIT_WAIT_FOR_NETWORK);
  }
  else if (!modem.isGprsConnected())
  {
    change_state(MODEM_INIT_APN);
  }
  else if (!mqtt.connected())
  {
    change_state(MODEM_INIT_MQTT);
  }
}

void Comm::check_power_condition()
{
  if (m_battery->getBatteryVoltage() < m_battery_on_voltage - m_battery_treshold_voltage)
  {
    change_state(MODEM_POWER_OFF);
  }
  else if (false == requested_power_on)
  {
    change_state(MODEM_POWER_OFF);
  }
}

void Comm::publish_init_message()
{

  String modem_name = "\"modem_name\":\"" + m_modem_name + "\"";

  String modem_info = "\"modem_info\":\"" + m_modem_info + "\"";

  String timestamp = "\"timestamp\":\"" + get_time() + "\"";

  String json = "{\n  " +
                timestamp + ",\n  " +
                modem_name + ",\n  " +
                modem_info + "\n" +
                "}";
  String topic = build_measure_topic("service", m_deviceId + "_modem");
  Serial.println("send data to topic: " + topic);
  mqtt.publish(topic.c_str(), json.c_str());
}

String Comm::build_topic(String lower_elements)
{
  return "irvine/" + m_deviceId + "/" + lower_elements;
}

String Comm::build_measure_topic(const String &sensor_type, const String &sensor_address)
{
  return "irvine/" + m_deviceId + "/" + sensor_type + "/" + sensor_address;
}

void Comm::change_state(state_e new_state)
{
  switch (new_state)
  {
  case MODEM_BAT_EN:
    m_sm_timestamp = millis();
    break;
  case MODEM_RESET:
    m_sm_timestamp = millis();
    break;
  case MODEM_POWER_UP:
    m_sm_timestamp = millis();
    break;
  case MODEM_POWER_UP_DELAY:
    m_sm_timestamp = millis();
    break;
  case MODEM_INIT_FAIL_DELAY:
    m_sm_timestamp = millis();
    break;
  case MODEM_INIT_WAIT_FOR_NETWORK:
    m_sm_timestamp = millis();
    break;
  case MODEM_INIT_WAIT_FOR_MQTT:
    m_sm_timestamp = millis();
    break;
  case MODEM_POWER_OFF:
    out_connected = false;
    out_powered_up = false;
    break;
  case MODEM_INIT:
    out_powered_up = true;
    break;
  case MODEM_POWER_ON:
    out_connected = true;
    break;
  }

  m_state = new_state;
}

bool Comm::publish_measure_data(const String &type, const String &sensor, const String &payload)
{
  if (!out_connected)
  {
    return false;
  }
  char buf[200];
  const String timestamp = get_time();
  sprintf(buf, "{\"timestamp\":\"%s\",\"value\":\"%s\"}",
          timestamp.c_str(), payload.c_str());

  mqtt.publish(build_measure_topic(type, sensor).c_str(), buf);

  return true;
}

String Comm::get_time(void)
{
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  float timezone;
  bool result = modem.getNetworkTime(&year, &month, &day, &hour, &minute, &second, &timezone);

  if (!result)
  {
    return String();
  }

  char buf[30];
  sprintf(buf, "%4u-%02u-%02u %02u:%02u:%02u GMT%+.0f", year, month, day, hour, minute, second, timezone);

  return String(buf);
}

String &Comm::getDeviceId()
{
  return m_deviceId;
}

void Comm::mqtt_callback(const String &topic, const String &message)
{
}

void Comm::mqtt_service_callback(const String &type, const String &payload)
{
}

void Comm::mqtt_irvine_callback(const String &type, const String &payload)
{
}
