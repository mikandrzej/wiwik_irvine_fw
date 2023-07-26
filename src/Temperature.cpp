#include "Temperature.h"

void Temperature::setup()
{
    m_oneWire = OneWire(ONE_WIRE_BUS);
    m_sensors = DallasTemperature(&m_oneWire);

    m_sensors.begin();

    m_sensors.setWaitForConversion(false);
}

void Temperature::loop()
{
    switch (m_sensorsState)
    {
    case SENSORS_STATE_IDLE:
        m_sensorsState = SENSORS_STATE_TRIGGER;
        m_period_timestamp = millis();
        break;
    case SENSORS_STATE_TRIGGER:
        m_sensors.requestTemperatures();
        m_sensorsState = SENSORS_STATE_WAITING;
        break;
    case SENSORS_STATE_WAITING:
        if (m_sensors.isConversionComplete())
        {
            m_sensorsState = SENSORS_STATE_GET_TEMPERATURE;
        }
        break;
    case SENSORS_STATE_GET_TEMPERATURE:
        m_tempC = m_sensors.getTempCByIndex(0);
        if (m_tempC != DEVICE_DISCONNECTED_C)
        {
            m_onTemperatureReady(m_tempC);
        }
        else
        {
            Serial.println("Error: Could not read temperature data");
        }
        m_sensorsState = SENSORS_STATE_DELAY;
        break;
    case SENSORS_STATE_DELAY:
        uint32_t t = millis();
        uint32_t diff = t - m_period_timestamp;
        if (diff > m_period)
        {
            m_period_timestamp = t;
            m_sensorsState = SENSORS_STATE_TRIGGER;
        }
        break;
    }
}

void Temperature::setMeasurePeriod(uint32_t period_ms)
{
    m_period = period_ms;
}

void Temperature::setOnTemperatureReady(const std::function<void(float)> &newOnTemperatureReady)
{
    m_onTemperatureReady = newOnTemperatureReady;
}