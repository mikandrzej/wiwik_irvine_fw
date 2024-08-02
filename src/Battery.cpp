#include "Comm.h"
#include "IrvineConfiguration.h"
#include "Battery.h"

#define BAT_ADC 35

static float getVoltage(const uint8_t pin);

void Battery::loop()
{
    uint32_t t = millis();
    uint32_t diff;

    diff = t - m_last_battery_shot;
    if (diff > irvineConfiguration.device.batteryInterval)
    {
        m_last_battery_shot = t;

        float voltage = getVoltage(BAT_ADC);

        Serial.println("Battery voltage: " + String(voltage));
        if (m_onBatteryVoltageReady)
        {
            m_onBatteryVoltageReady(voltage);
        }
    }
}

static float getVoltage(const uint8_t pin)
{
    int vref = 1100;
    uint16_t volt = analogRead(pin);
    float battery_voltage = ((float)volt / 4095.0) * 2.0 * 3.3 * (vref);
    return battery_voltage;
}

void Battery::setOnBatteryVoltageReady(const std::function<void(float)> &newOnBatteryVoltageReady)
{
    m_onBatteryVoltageReady = newOnBatteryVoltageReady;
}

float Battery::getBatteryVoltage()
{
    return getVoltage(BAT_ADC);
}