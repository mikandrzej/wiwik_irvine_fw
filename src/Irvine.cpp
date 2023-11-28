#include "Irvine.h"

#include <Wire.h>

void Irvine::loop()
{
    if (initSm())
    {
        m_comm.loop();

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
    m_temperature.setMeasurePeriod(10000u);
    m_temperature.setOnTemperatureReady(
        [this](String &sensorAddress, float temperature)
        {
            this->onTemperatureReady(sensorAddress, temperature);
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

    m_comm.setBattery(&m_battery);

    return true;
}

boolean Irvine::gpsInit()
{
    m_gps.setGpsPeriod(5000u);
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
    m_comm.publish_measure_data("temperature", sensorAddress, String(temperature));
}

void Irvine::onBatteryVoltageReady(float voltage)
{
    String sensor_id = m_comm.getDeviceId() + "_battery";
    m_comm.publish_measure_data("battery", sensor_id, String(voltage));
}

void Irvine::onGpsDataReady(GpsData &gpsData)
{
    String sensor_id = m_comm.getDeviceId() + "_gps";
    m_comm.publish_measure_data("gps", sensor_id, gpsData.m_raw);
}