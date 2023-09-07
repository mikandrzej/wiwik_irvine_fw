#pragma once

#include <Arduino.h>

class Battery
{
    public:
        void setup();
        void loop();
        void setBatteryPeriod(const uint32_t period);
        void setOnBatteryVoltageReady(const std::function<void(float)> &newOnBatteryVoltageReady);
        float getBatteryVoltage();

    private:
        uint32_t m_last_battery_shot;
        uint32_t m_battery_period = 5000;

        std::function<void(float)> m_onBatteryVoltageReady;
};