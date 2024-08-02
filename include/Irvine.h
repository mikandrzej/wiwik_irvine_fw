#pragma once

#include <Arduino.h>

#include "Battery.h"
#include "Comm.h"
#include "Gps.h"
#include <DataLogger.h>
#include "UdsQueryManager.h"

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
        INIT_STATE_UDS_INIT,
        INIT_STATE_DONE
    } m_initState;

    Battery m_battery;
    Gps m_gps;
    UdsQueryManager m_udsQueryManager;


    const float m_battery_treshold = 2900.0f;

    boolean initSm();
    boolean batteryInit();
    boolean gpsInit();
    boolean udsInit();

    void onTemperatureReady(String &sensorAddress, float temperature);
    void onBatteryVoltageReady(float voltage);
    void onGpsDataReady(GpsData &gpsData);
    void onTankLevel(const float value);

    DataLogger temperatureLogger = DataLogger("temp");
    DataLogger batteryLogger = DataLogger("battery");
    DataLogger gpsLogger = DataLogger("gps");
};