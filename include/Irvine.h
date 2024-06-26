#pragma once

#include <Arduino.h>

#include "Battery.h"
#include "Comm.h"
#include "Gps.h"
#include "BLEJaaleTemperature.h"

class Irvine
{
public:
    void loop();

// private:
    const char *m_pin = "0000";

    String m_topicUserId = "";
    String m_topicMeasures = "";

    String m_userId = "";

    const uint32_t m_mqttConnectTimeout = 5000u;
    uint32_t m_mqttConnectTimestamp = 0u;

    enum
    {
        INIT_STATE_BATTERY_INIT,
        INIT_STATE_GPS_INIT,
        INIT_STATE_DONE
    } m_initState;

    Battery m_battery;
    Gps m_gps;

    const float m_battery_treshold = 2900.0f;

    boolean initSm();
    boolean batteryInit();
    boolean gpsInit();

    void onTemperatureReady(String &sensorAddress, float temperature);
    void onBatteryVoltageReady(float voltage);
    void onGpsDataReady(GpsData &gpsData);
};