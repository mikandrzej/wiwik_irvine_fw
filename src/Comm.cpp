#include "Battery.h"
#include "Comm.h"
#include <WiFi.h>

#include "EgTinyGsm.h"

#define TINY_GSM_MODEM_SIM7600 // A7670's AT instruction is compatible with SIM7600
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <TinyGPSPlus.h>
#include <ArduinoHttpClient.h>

#include <Update.h>
#include <StreamString.h>

#include "Configuration.h"

#define SerialAT Serial1
#define UART_BAUD 115200
#define PIN_TX 19
#define PIN_RX 18

#define PWR_PIN 21
#define RESET 16

extern uint32_t software_version;

EgTinyGsm modem(SerialAT);
TinyGsmClient client(modem, 0);
TinyGsmClient client2(modem, 1);
PubSubClient mqtt(client);

const char server[] = "cloud.2canit.pl";
const char resource[] = "/firmware.bin";
const int port = 80;
static HttpClient http(client2, server, port);

TinyGPSPlus gps;

Comm comm;

static void blink(int times)
{
  return;
  for (int k = 0; k < times; k++)
  {
    digitalWrite(22, 1);
    delay(200);
    digitalWrite(22, 0);
    delay(200);
  }
  delay(300);
}

void Comm::loop()
{
  check_power_condition();

  if (m_state < MODEM_POWER_ON)
    blink((int)m_state);

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

  pinMode(RESET, OUTPUT);
  pinMode(PWR_PIN, OUTPUT);

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
    // if (m_battery->getBatteryVoltage() > (m_battery_on_voltage + m_battery_treshold_voltage))
    // {
    change_state(MODEM_POWER_DOWN);

    Serial.println("Modem power down");
    // }
  }
}

void Comm::state_modem_power_down()
{
  digitalWrite(RESET, HIGH);
  digitalWrite(PWR_PIN, HIGH);

  change_state(MODEM_RESET);

  Serial.println("Modem bat enable");
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
  if (millis() - m_sm_timestamp > 20000)
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
    for (int k = 0; k < 7; k++)
    {
      digitalWrite(22, 1);
      delay(100);
      digitalWrite(22, 0);
      delay(100);
    }
  }
}

void Comm::state_modem_init_sim_unlock()
{
  SimStatus sim_status = modem.getSimStatus(2000);
  if (sim_status == SIM_LOCKED)
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
  else if (sim_status == SIM_READY)
  {
    Serial.println("PIN code is not enabled");
    change_state(MODEM_INIT_WAIT_FOR_NETWORK);
  }
  else
  {
    Serial.println("SIM ERROR: " + String(sim_status));
    change_state(MODEM_INIT_FAIL_DELAY);
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
  else if (millis() - m_sm_timestamp > 120000)
  {
    Serial.println("Network not found - reset");
    change_state(MODEM_INIT_FAIL_DELAY);
  }
}

void Comm::state_modem_init_apn()
{
  if (modem.isNetworkConnected())
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
  else
  {
    Serial.println("Network lost during GPRS connecting");
    change_state(MODEM_INIT_WAIT_FOR_NETWORK);
  }
}

void Comm::state_modem_init_gps()
{
  bool result = false;

  do
  {
    modem.sendAT("+CGNSSPWR=0");
    if (modem.waitResponse(1000) != 1)
    {
      break;
    }
    modem.sendAT("+CGNSSPWR=1");
    if (modem.waitResponse(1000) != 1)
    {
      break;
    }
    if (modem.waitResponse(20000UL, "+CGNSSPWR: READY!") != 1)
    {
      break;
    }
    modem.sendAT("+CGNSSMODE=3");
    if (modem.waitResponse(1000) != 1)
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
  else
  {
    Serial.println("GPS init fail - reset");
    change_state(MODEM_INIT_FAIL_DELAY);
  }
}

void Comm::state_modem_init_mqtt()
{
  static String url = configuration.getMqttServerAddress();
  uint16_t port = configuration.getMqttServerPort();
  Serial.printf("Setting MQTT server to: %s:%d\r\n", url.c_str(), port);
  mqtt.setServer(url.c_str(), port);

  Serial.println("Waiting for MQTT");

  change_state(MODEM_INIT_WAIT_FOR_MQTT);
}

void Comm::state_modem_init_wait_for_mqtt()
{
  static String login = configuration.getMqttServerLogin();
  static String pass = configuration.getMqttServerPassword();

  bool result;

  if (modem.isGprsConnected())
  {
    if (login.length() > 0)
    {
      Serial.printf("Connecting to MQTT: %s / %s\r\n", login.c_str(), pass.c_str());
      result = mqtt.connect(m_deviceId.c_str(),
                            login.c_str(),
                            pass.c_str());
    }
    else
    {
      Serial.printf("Connecting to MQTT without login\r\n");
      result = mqtt.connect(m_deviceId.c_str());
    }
    if (result)
    {
      Serial.println("MQTT Connected - sending hello message");

      mqtt.setCallback([this](char *topic, uint8_t *payload, unsigned int len)
                       { this->mqtt_callback(String(topic), String(payload, len)); });
      static String subscribed_topic = build_topic("service/to_device/#");
      mqtt.subscribe(subscribed_topic.c_str());

      publish_init_message();

      change_state(MODEM_POWER_ON);
    }
  }
  else
  {
    Serial.println("GPRS disconnected during MQTT connecting");
    change_state(MODEM_INIT_APN);
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
  else
  {
    mqtt.loop();
    if (millis() - m_lastNetworkShot > configuration.getNetworkReportInterval())
    {
      m_lastNetworkShot = millis();
      String sensor_id = comm.getDeviceId() + "_network";
      DynamicJsonDocument doc(200);

      doc["op"] = modem.getOperator();
      doc["signal"] = modem.getSignalQuality();
      doc["mode"] = modem.getNetworkMode();
      doc["temp"] = modem.getTemperature();

      char buf[200];
      serializeJson(doc, buf, sizeof(buf));
      Serial.println(buf);
      comm.publish_measure_data("service", sensor_id, doc);
    }
  }
}

void Comm::check_power_condition()
{
  // if (m_battery->getBatteryVoltage() < m_battery_on_voltage - m_battery_treshold_voltage)
  // {
  //   change_state(MODEM_POWER_OFF);
  //   for (int k = 0; k < 3; k++)
  //   {
  //     digitalWrite(22, 1);
  //     delay(100);
  //     digitalWrite(22, 0);
  //     delay(100);
  //   }
  // }
  // else
  if (false == requested_power_on)
  {
    change_state(MODEM_POWER_OFF);
    for (int k = 0; k < 5; k++)
    {
      digitalWrite(22, 1);
      delay(100);
      digitalWrite(22, 0);
      delay(100);
    }
  }
}

void Comm::publish_init_message()
{
  DynamicJsonDocument jsonDocument(300);
  char buf[550];
  jsonDocument["name"] = modem.getModemName();
  jsonDocument["ccid"] = modem.getSimCCID();
  jsonDocument["imsi"] = modem.getIMSI();
  jsonDocument["imei"] = modem.getIMEI();

  size_t len = serializeJson(jsonDocument, buf, sizeof(buf));

  String sensor = m_deviceId + "_modem";
  String topic = build_measure_topic("service", sensor);
  mqtt.publish(topic.c_str(), buf, len);
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

bool Comm::publish_measure_data(const String &type, const String &sensor, const float value)
{
  if (!out_connected)
  {
    return false;
  }
  DynamicJsonDocument jsonDocument(200);
  char buf[256];
  jsonDocument["timestamp"] = get_time();
  jsonDocument["value"] = value;

  size_t len = serializeJson(jsonDocument, buf, sizeof(buf));

  if (len > 0u)
  {
    mqtt.publish(build_measure_topic(type, sensor).c_str(), buf);
  }
  else
  {
    Serial.println("Deserialization error");
  }

  return true;
}

bool Comm::publish_measure_data(const String &type, const String &sensor, DynamicJsonDocument &jsonDocument)
{
  if (!out_connected)
  {
    return false;
  }
  jsonDocument["timestamp"] = get_time();

  char buf[256];
  size_t len = serializeJson(jsonDocument, buf, sizeof(buf));
  if (len > 0u)
  {
    mqtt.publish(build_measure_topic(type, sensor).c_str(), buf);
  }
  else
  {
    Serial.println("Deserialization error");
  }

  return true;
}

void Comm::publish_debug_data(const String &message)
{
  mqtt.publish(build_measure_topic("debug", m_deviceId + "_debug").c_str(), message.c_str());
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
  Serial.println("Rcv mqtt data: " + topic + ": " + message);
  String service_topic = build_topic("service/to_device");
  if (topic.startsWith(service_topic))
  {
    String type = topic.substring(service_topic.length() + 1);
    mqtt_service_callback(type, message);
  }
}

void Comm::mqtt_service_callback(const String &type, const String &payload)
{
  Serial.println(("Rcv service data: " + type + ": " + payload).c_str());
  if (type == "get_config")
  {
    mqtt.publish(build_topic("service/config").c_str(), "config will be here");
  }
  else if (type == "set_config")
  {
    DynamicJsonDocument jsonDocument(200);
    DeserializationError err = deserializeJson(jsonDocument, payload.c_str());
    if (err == DeserializationError::Ok)
    {
      if (jsonDocument.containsKey("mqtt_server_address"))
      {
        String mqtt_server((const char *)jsonDocument["mqtt_server_address"]);
        Serial.printf("Setting mqtt_server_address to: %s\r\n", mqtt_server.c_str());
        configuration.setMqttServerAddress(mqtt_server);
      }
      if (jsonDocument.containsKey("mqtt_server_port"))
      {
        uint16_t port = jsonDocument["mqtt_server_port"].as<uint16_t>();
        Serial.printf("Setting mqtt_server_port to: %u\r\n", port);
        configuration.setMqttServerPort(port);
      }
      if (jsonDocument.containsKey("mqtt_server_login"))
      {
        String mqtt_login((const char *)jsonDocument["mqtt_server_login"]);
        Serial.printf("Setting mqtt_server_login to: %s\r\n", mqtt_login.c_str());
        configuration.setMqttServerLogin(mqtt_login);
      }
      if (jsonDocument.containsKey("mqtt_server_password"))
      {
        String mqtt_password((const char *)jsonDocument["mqtt_server_password"]);
        Serial.printf("Setting mqtt_server_password to: %s\r\n", mqtt_password.c_str());
        configuration.setMqttServerPassword(mqtt_password);
      }
      if (jsonDocument.containsKey("temperature_interval"))
      {
        uint32_t interval = jsonDocument["temperature_interval"].as<uint32_t>();
        Serial.printf("Setting temperature_interval to: %u\r\n", interval);
        configuration.setTemperatureReportInterval(interval);
      }
      if (jsonDocument.containsKey("gps_interval"))
      {
        uint32_t interval = jsonDocument["gps_interval"].as<uint32_t>();
        Serial.printf("Setting gps_interval to: %u\r\n", interval);
        configuration.setGpsReportInterval(interval);
      }
      if (jsonDocument.containsKey("battery_interval"))
      {
        uint32_t interval = jsonDocument["battery_interval"].as<uint32_t>();
        Serial.printf("Setting battery_interval to: %u\r\n", interval);
        configuration.setBatteryReportInterval(interval);
      }
      if (jsonDocument.containsKey("debug_mode"))
      {
        uint8_t mode = jsonDocument["debug_mode"].as<uint8_t>();
        Serial.printf("Setting debug_mode to: %u\r\n", mode);
        configuration.setDebugMode(mode);
      }
      if (jsonDocument.containsKey("jaalee_address"))
      {
        String jaalee_address((const char *)jsonDocument["jaalee_address"]);
        Serial.printf("Setting jaalee_address to: %s\r\n", jaalee_address.c_str());
        configuration.setJaaleSensorAddress(jaalee_address);
      }
      mqtt.publish(build_topic("service/set_config_response").c_str(), "Configuration saved");
    }
    else
    {
      mqtt.publish(build_topic("service/set_config_response").c_str(),
                   (String("Pasing error: ") + String(err.c_str())).c_str());
    }
  }
  else if (type == "reboot")
  {
    Serial.println("Reboot in 10 seconds");
    mqtt.publish(build_topic("service/reboot_response").c_str(), "Reboot in 3 seconds");

    delay(3000);

    ESP.restart();
  }
  else if (type == "update")
  {
    DynamicJsonDocument jsonDocument(200);
    DeserializationError err = deserializeJson(jsonDocument, payload.c_str());

    if (jsonDocument.containsKey("sw_ver"))
    {
      uint32_t remote_sw_version = jsonDocument["sw_ver"].as<uint32_t>();
      if (remote_sw_version > software_version)
      {
        Serial.println("Performing software update");
        if (jsonDocument.containsKey("sw_server") && jsonDocument.containsKey("sw_port") && jsonDocument.containsKey("sw_path"))
        {
          String sw_server = String((const char *)jsonDocument["sw_server"]);
          uint16_t sw_port = jsonDocument["sw_port"].as<uint16_t>();
          String sw_path = String((const char *)jsonDocument["sw_path"]);

          Serial.print("Software URL: ");
          Serial.println((sw_server + ":" + String(sw_port) + sw_path).c_str());
          mqtt.publish(build_topic("service/update_response").c_str(), "Update started");

          http.connect(sw_server.c_str(), sw_port);
          int err = http.get(sw_path);
          if (err == 0)
          {
            int status = http.responseStatusCode();
            if (status == 200)
            {
              int length = http.contentLength();
              Serial.printf("Content length: %d\r\n", length);
              if (length > 0)
              {
                int _lastError = 0;
                StreamString error;
                if (!Update.begin(length, U_FLASH))
                {
                  _lastError = Update.getError();
                  Update.printError(error);
                  error.trim(); // remove line ending
                  mqtt.publish(build_topic("service/update_response").c_str(), ("Update error: " + error).c_str());
                }

                // if (md5.length())
                // {
                //   if (!Update.setMD5(md5.c_str()))
                //   {
                //     _lastError = HTTP_UE_SERVER_FAULTY_MD5;
                //     DEBUG_HTTP_UPDATE("[httpUpdate] Update.setMD5 failed! (%s)\n", md5.c_str());
                //     return false;
                //   }
                // }

                else if (Update.writeStream(http) != length)
                {
                  _lastError = Update.getError();
                  Update.printError(error);
                  error.trim(); // remove line ending
                  mqtt.publish(build_topic("service/update_response").c_str(), ("Update error: " + error).c_str());
                }
                else if (!Update.end())
                {
                  _lastError = Update.getError();
                  Update.printError(error);
                  error.trim(); // remove line ending
                  mqtt.publish(build_topic("service/update_response").c_str(), ("Update error: " + error).c_str());
                }
                else
                {
                  mqtt.publish(build_topic("service/update_response").c_str(), "Update succeed. Send reboot command");
                }
              }
              else
              {
                Serial.printf("Invalid update data length: %d\r\n", length);
                mqtt.publish(build_topic("service/update_response").c_str(), ("HTTP data length invalid: " + String(length)).c_str());
              }
            }
            else
            {
              Serial.printf("Http request status code: %d\r\n", status);
              mqtt.publish(build_topic("service/update_response").c_str(), ("HTTP request status code: " + String(status)).c_str());
            }
          }
          else
          {
            Serial.printf("Http request error code: %d\r\n", err);
            mqtt.publish(build_topic("service/update_response").c_str(), ("HTTP request error code: " + String(err)).c_str());
          }
        }
        else
        {
          Serial.println("Abort sw update, no url");
          mqtt.publish(build_topic("service/update_response").c_str(), "Update error - json should contain sw_server, sw_port, sw_path keys");
        }
      }
      else
      {
        Serial.println("Performing software update");
        String sw_ver = String(software_version);
        mqtt.publish(build_topic("service/update_response").c_str(), ("Update not expected. Installed version: " + sw_ver).c_str());
      }
    }
  }
}

void Comm::mqtt_irvine_callback(const String &type, const String &payload)
{
}
