#pragma once

#include <Arduino.h>

class Battery
{
    public:
        void setup();
        void loop();
        void setOnBatteryVoltageReady(const std::function<void(float)> &newOnBatteryVoltageReady);
        float getBatteryVoltage();

    private:
        uint32_t m_last_battery_shot;

        std::function<void(float)> m_onBatteryVoltageReady;
};