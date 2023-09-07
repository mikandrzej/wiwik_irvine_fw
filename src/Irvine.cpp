#include "Irvine.h"

#define TINY_GSM_MODEM_SIM7600 // A7670's AT instruction is compatible with SIM7600
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <Wire.h>

#define SerialAT Serial1
#define PWR_PIN 4
#define RESET 5
#define BAT_EN 12
#define MODEM_FLIGHT 25

#define UART_BAUD 115200
#define PIN_TX 26
#define PIN_RX 27

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

void Irvine::loop()
{
    if (initSm())
    {
        mqtt.loop();

        m_temperature.loop();

        m_battery.loop();
    }
}

boolean Irvine::initSm()
{
    switch (m_initState)
    {
    case INIT_STATE_TEMPERATURE_INIT:
        if (temperatureInit())
        {
            m_initState = INIT_STATE_BATTERY_INIT;
        }
        else
        {
            m_initState = INIT_STATE_TEMPERATURE_INIT;
        }
        break;
    case INIT_STATE_BATTERY_INIT:
        if (batteryInit())
        {
            m_initState = INIT_STATE_MODEM_RESET;
        }
        else
        {
            m_initState = INIT_STATE_TEMPERATURE_INIT;
        }
        break;
    case INIT_STATE_MODEM_RESET:
        if (modemReset())
        {
            m_initState = INIT_STATE_MODEM_INIT;
        }
        else
        {
            m_initState = INIT_STATE_TEMPERATURE_INIT;
        }
        break;
    case INIT_STATE_MODEM_INIT:
        if (modemInit())
        {
            m_initState = INIT_STATE_SIM_UNLOCK;
        }
        else
        {
            m_initState = INIT_STATE_TEMPERATURE_INIT;
        }
        break;
    case INIT_STATE_SIM_UNLOCK:
        if (simUnlock())
        {
            m_initState = INIT_STATE_WAIT_FOR_NETWORK;
        }
        else
        {
            m_initState = INIT_STATE_TEMPERATURE_INIT;
        }
        break;
    case INIT_STATE_WAIT_FOR_NETWORK:
        if (waitForNetwork())
        {
            m_initState = INIT_STATE_APN_CONNECT;
        }
        else
        {
            m_initState = INIT_STATE_TEMPERATURE_INIT;
        }
        break;
    case INIT_STATE_APN_CONNECT:
        if (apnConnect())
        {
            m_initState = INIT_STATE_MQTT_CONNECT;
        }
        else
        {
            m_initState = INIT_STATE_TEMPERATURE_INIT;
        }
        break;
    case INIT_STATE_MQTT_CONNECT:
        if (mqttConnect())
        {
            m_initState = INIT_STATE_DONE;
        }
        else
        {
            m_initState = INIT_STATE_TEMPERATURE_INIT;
        }
        break;
    case INIT_STATE_DONE:
        initDone();
        return true;
    }

    return false;
}

boolean Irvine::temperatureInit()
{
    m_temperature.setup();
    m_temperature.setMeasurePeriod(10000u);
    m_temperature.setOnTemperatureReady(
        [this](float temperature)
        {
            this->onTemperatureReady(temperature);
        });

    return true;
}

boolean Irvine::batteryInit()
{
    m_battery.setBatteryPeriod(5000u);
    m_battery.setOnBatteryVoltageReady(
        [this](float voltage)
        {
            this->onBatteryVoltageReady(voltage);
        });

    return true;
}

boolean Irvine::modemReset()
{
    SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

    if (m_battery.getBatteryVoltage() < m_battery_treshold)
    {
        return false;
    }

    pinMode(BAT_EN, OUTPUT);
    pinMode(RESET, OUTPUT);
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(BAT_EN, LOW);
    delay(1000);
    digitalWrite(BAT_EN, HIGH);

    // A7670 Reset
    digitalWrite(RESET, LOW);
    delay(100);
    digitalWrite(RESET, HIGH);
    delay(3000);
    digitalWrite(RESET, LOW);

    // Launch SIM7000
    digitalWrite(PWR_PIN, HIGH);
    delay(300);
    digitalWrite(PWR_PIN, LOW);

    Serial.println("\nWait after modem reset...");

    delay(10000);

    return true;
}

boolean Irvine::modemInit()
{
    DBG("Initializing modem...");
    if (!modem.init())
    {
        DBG("Failed to restart modem, delaying 10s and retrying");
        delay(10000);
        return false;
    }

    String name = modem.getModemName();
    DBG("Modem Name:", name);

    String modemInfo = modem.getModemInfo();
    DBG("Modem Info:", modemInfo);
    return true;
}

boolean Irvine::simUnlock()
{
    // Unlock your SIM card with a PIN if needed
    if (modem.getSimStatus() != 3)
    {
        modem.simUnlock(m_pin);
    }
    return true;
}

boolean Irvine::waitForNetwork()
{
    SerialMon.print("Waiting for network...");
    if (!modem.waitForNetwork())
    {
        SerialMon.println(" fail");
        delay(10000);
        return false;
    }
    SerialMon.println(" success");

    if (modem.isNetworkConnected())
    {
        SerialMon.println("Network connected");
        return true;
    }
    return false;
}

boolean Irvine::apnConnect()
{
    // GPRS connection parameters are usually set after network registration
    SerialMon.print(F("Connecting to "));
    SerialMon.print(m_apn);
    if (!modem.gprsConnect(m_apn, m_apnUsername, m_apnPassword))
    {
        SerialMon.println(" fail");
        delay(10000);
        return false;
    }
    SerialMon.println(" success");

    if (modem.isGprsConnected())
    {
        SerialMon.println("GPRS connected");
        return true;
    }
    return false;
}

boolean Irvine::mqttConnect()
{
    uint32_t t = millis();
    uint32_t diff = t - m_mqttConnectTimestamp;
    if (diff > m_mqttConnectTimeout || m_mqttConnectTimestamp == 0u)
    {
        if (m_mqttConnectTimestamp == 0u)
        {
            SerialMon.println("Setting MQTT data");

            mqtt.setServer(m_broker, 1883);
            mqtt.setCallback(
                [this](char *topic, uint8_t *payload, unsigned int len)
                { this->mqttCallback(topic, payload, len); });
        }

        m_mqttConnectTimestamp = t;

        if (mqtt.connect(m_deviceId.c_str()))
        {
            SerialMon.println("Mqtt connected!");
            mqtt.publish("wiwik_irvine_fw", "hello world");
            SerialMon.println("Subscribing topic: " + m_topicUserId);
            mqtt.subscribe(m_topicUserId.c_str());

            return true;
        }

        SerialMon.println("Mqtt connect error!");
    }

    return false;
}

void Irvine::initDone()
{
    // Make sure we're still registered on the network
    if (!modem.isNetworkConnected())
    {
        SerialMon.println("Network disconnected!");
        m_initState = INIT_STATE_WAIT_FOR_NETWORK;
    }
    else if (!modem.isGprsConnected())
    {
        SerialMon.println("GPRS disconnected!");
        m_initState = INIT_STATE_APN_CONNECT;
    }
    else if (!mqtt.connected())
    {
        SerialMon.println("MQTT disconnected!");
        m_initState = INIT_STATE_MQTT_CONNECT;
    }
}

void Irvine::mqttCallback(char *topic, uint8_t *payload, unsigned int len)
{
    String topicStr(topic);
    String msg(payload, len);
    SerialMon.println("MQTT callback topic: " + topicStr + " msg: " + msg);

    if (0 == topicStr.compareTo(m_topicUserId))
    {
        SerialMon.println("User ID received: " + msg);
        setUserId(msg);
    }
}

void Irvine::setDeviceId(String &devId)
{
    m_deviceId = devId;

    m_topicUserId = "devices/irvine/" + devId + "/user_id";
}

void Irvine::setUserId(String &userId)
{
    m_userId = userId;

    m_topicMeasures = "measures/" + userId + "/irvine/" + m_deviceId;

    mqtt.publish(m_topicMeasures.c_str(), (String("Hello ") + m_userId).c_str());
}

void Irvine::onTemperatureReady(float temperature)
{
    Serial.print("Temperature for the device 1 (index 0) is: ");
    Serial.println(temperature);

    String topic = m_topicMeasures + "/temperature1";
    String data(temperature);

    mqtt.publish(topic.c_str(), data.c_str());
}

void Irvine::onSolarVoltageReady(float voltage)
{
    Serial.print("Solar voltage is: ");
    Serial.println(voltage);

    String topic = m_topicMeasures + "/solar";
    String data(voltage);

    mqtt.publish(topic.c_str(), data.c_str());
}

void Irvine::onBatteryVoltageReady(float voltage)
{
    Serial.print("Battery voltage is: ");
    Serial.println(voltage);

    String topic = m_topicMeasures + "/battery";
    String data(voltage);

    mqtt.publish(topic.c_str(), data.c_str());
}