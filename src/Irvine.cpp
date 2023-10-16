#include "Irvine.h"

#include <Wire.h>

void Irvine::loop()
{
    if (initSm())
    {
        m_comm.loop();

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

    m_comm.setBattery(&m_battery);

    return true;
}

void Irvine::onTemperatureReady(float temperature)
{
    Serial.printf("temperature ready: %.2f\n", temperature);
    m_comm.publish_measure_data("temperature1", String(temperature));
}

void Irvine::onBatteryVoltageReady(float voltage)
{
    m_comm.publish_measure_data("battery", String(voltage));
}