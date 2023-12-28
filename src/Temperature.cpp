#include "Temperature.h"
#include "Configuration.h"

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
        {
            for (uint8_t idx = 0u; idx < SENSORS_MAX; idx++)
            {
                bool found = m_sensors.getAddress(address[idx], idx);
                if (!found)
                {
                    break;
                }

                m_tempC[idx] = m_sensors.getTempC(address[idx]);
                if (m_tempC[idx] <= DEVICE_DISCONNECTED_C)
                {
                    Serial.println("Error: 1Wire device has been disconnected during read");
                    break;
                }
                
                if(m_onTemperatureReady)
                {
                    String sensor_address = "dallas_0x";
                    for (uint8_t k = 0u; k < 8u; k++)
                    {
                        sensor_address += String(address[idx][k], HEX);
                    }
                    m_onTemperatureReady(sensor_address, m_tempC[idx]);
                }
            }
        }
        m_sensorsState = SENSORS_STATE_DELAY;
        break;
    case SENSORS_STATE_DELAY:
        uint32_t t = millis();
        uint32_t diff = t - m_period_timestamp;
        if (diff > configuration.getTemperatureReportInterval())
        {
            m_period_timestamp = t;
            m_sensorsState = SENSORS_STATE_TRIGGER;
        }
        break;
    }
}

void Temperature::setOnTemperatureReady(const std::function<void(String&, float)> &newOnTemperatureReady)
{
    m_onTemperatureReady = newOnTemperatureReady;
}