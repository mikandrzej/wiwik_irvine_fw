#pragma once

#include <Arduino.h>

class SimpleIntervalTimer
{
public:
    SimpleIntervalTimer(const uint32_t interval, const bool strict = false) : interval(interval), strict(strict)
    {
        lastShotTimestamp = millis();
    }

    bool check()
    {
        bool result = false;
        const uint32_t currTime = millis();
        if ((currTime - lastShotTimestamp) > interval)
        {
            result = true;
            if (strict)
            {
                lastShotTimestamp += interval;
            }
            else
            {
                lastShotTimestamp = currTime;
            }
        }
        return result;
    }

    void restart() { lastShotTimestamp = millis(); }

    void setInterval(const uint32_t interval) { this->interval = interval; }

private:
    uint32_t interval;
    uint32_t lastShotTimestamp;
    bool strict;
};