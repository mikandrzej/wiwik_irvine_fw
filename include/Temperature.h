#pragma once

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 21

#define SENSORS_MAX 5u

class Temperature
{
public:
    void setup();
    void loop();
    void setMeasurePeriod(uint32_t period_ms);
    void setOnTemperatureReady(const std::function<void(String&, float)> &newOnTemperatureReady);

private:
    OneWire m_oneWire;
    DallasTemperature m_sensors;
    uint32_t m_period_timestamp;

    float m_tempC[SENSORS_MAX];
    uint8_t address[SENSORS_MAX][8u];

    enum
    {
        SENSORS_STATE_IDLE,
        SENSORS_STATE_TRIGGER,
        SENSORS_STATE_WAITING,
        SENSORS_STATE_GET_TEMPERATURE,
        SENSORS_STATE_DELAY
    } m_sensorsState;

    std::function<void(String&, float)> m_onTemperatureReady;
};