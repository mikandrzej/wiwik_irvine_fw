#include "Irvine.h"

#include <Wire.h>


void Irvine::loop()
{
    if (initSm())
    {
        comm.loop();

        m_temperature.loop();

        m_battery.loop();

        m_gps.loop();
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
            m_initState = INIT_STATE_GPS_INIT;
        }
        else
        {
            m_initState = INIT_STATE_TEMPERATURE_INIT;
        }
        break;
    case INIT_STATE_GPS_INIT:
        if (gpsInit())
        {
            m_initState = INIT_STATE_DONE;
        }
        else
        {
            m_initState = INIT_STATE_TEMPERATURE_INIT;
        }
        break;
    case INIT_STATE_DONE:
        return true;
    }

    return false;
}

boolean Irvine::temperatureInit()
{
    m_temperature.setup();
    m_temperature.setOnTemperatureReady(
        [this](String &sensorAddress, float temperature)
        {
            this->onTemperatureReady(sensorAddress, temperature);
        });

    return true;
}

boolean Irvine::batteryInit()
{
    m_battery.setOnBatteryVoltageReady(
        [this](float voltage)
        {
            this->onBatteryVoltageReady(voltage);
        });

    comm.setBattery(&m_battery);

    return true;
}

boolean Irvine::gpsInit()
{
    m_gps.setOnGpsDataReady(
        [this](GpsData &gpsData)
        {
            this->onGpsDataReady(gpsData);
        });
    return true;
}

void Irvine::onTemperatureReady(String &sensorAddress, float temperature)
{
    Serial.printf("temperature ready for %s: %.2f\n", sensorAddress.c_str(), temperature);
    comm.publish_measure_data("temperature", sensorAddress, temperature);
}

void Irvine::onBatteryVoltageReady(float voltage)
{
    String sensor_id = comm.getDeviceId() + "_battery";
    comm.publish_measure_data("battery", sensor_id, voltage);
}

void Irvine::onGpsDataReady(GpsData &gpsData)
{
    String sensor_id = comm.getDeviceId() + "_gps";
    DynamicJsonDocument doc(200);
    doc["tim"] = gpsData.m_timestamp;
    doc["lng"] = gpsData.m_longitude;
    doc["lat"] = gpsData.m_latitude;
    doc["alt"] = gpsData.m_altitude;
    doc["acc"] = gpsData.m_precision;
    doc["sat"] = gpsData.m_satellites;
    doc["spd"] = gpsData.m_speed;
    char buf[200];
    serializeJson(doc, buf, sizeof(buf));
    Serial.println(buf);
    comm.publish_measure_data("gps", sensor_id, doc);
}
