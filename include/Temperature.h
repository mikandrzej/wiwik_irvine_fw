#pragma once

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 32

class Temperature
{
public:
    void setup();
    void loop();
    void setMeasurePeriod(uint32_t period_ms);
    void setOnTemperatureReady(const std::function<void(float)> &newOnTemperatureReady);

private:
    OneWire m_oneWire;
    DallasTemperature m_sensors;
    uint32_t m_period_timestamp;
    uint32_t m_period;

    float m_tempC;

    enum
    {
        SENSORS_STATE_IDLE,
        SENSORS_STATE_TRIGGER,
        SENSORS_STATE_WAITING,
        SENSORS_STATE_GET_TEMPERATURE,
        SENSORS_STATE_DELAY
    } m_sensorsState;

    std::function<void(float)> m_onTemperatureReady;
};