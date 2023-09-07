#pragma once

#include <Arduino.h>

#include "Temperature.h"
#include "Battery.h"

class Irvine
{
public:
    void loop();
    void setDeviceId(String &devId);

private:
    const char *m_pin = "0000";

    const char *m_apn = "internet";
    const char *m_apnUsername = "internet";
    const char *m_apnPassword = "internet";

    const char *m_broker = "iot.2canit.pl";
    String m_deviceId = "";
    String m_topicUserId = "";
    String m_topicMeasures = "";

    String m_userId = "";

    const uint32_t m_mqttConnectTimeout = 5000u;
    uint32_t m_mqttConnectTimestamp = 0u;

    enum
    {
        INIT_STATE_TEMPERATURE_INIT,
        INIT_STATE_BATTERY_INIT,
        INIT_STATE_MODEM_RESET,
        INIT_STATE_MODEM_INIT,
        INIT_STATE_SIM_UNLOCK,
        INIT_STATE_WAIT_FOR_NETWORK,
        INIT_STATE_APN_CONNECT,
        INIT_STATE_MQTT_CONNECT,
        INIT_STATE_DONE
    } m_initState;

    Temperature m_temperature;
    Battery m_battery;

    const float m_battery_treshold = 2900.0f;

    boolean initSm();
    boolean temperatureInit();
    boolean batteryInit();
    boolean modemReset();
    boolean modemInit();
    boolean simUnlock();
    boolean waitForNetwork();
    boolean apnConnect();
    boolean mqttConnect();
    void initDone();
    void setUserId(String &userId);


    void mqttCallback(char *topic, uint8_t *payload, unsigned int len);
    void onTemperatureReady(float temperature);
    void onSolarVoltageReady(float voltage);
    void onBatteryVoltageReady(float voltage);
};