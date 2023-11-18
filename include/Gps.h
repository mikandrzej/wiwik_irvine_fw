#pragma once

#include <Arduino.h>

struct GpsData 
{
    double speed;
    int latitude;
    int longitude;
    String raw;
};

class Gps
{
public:
    void setup();
    void loop();
    void setGpsPeriod(const uint32_t period);
    void setGpsMinDistance(const uint32_t meters);
    void setOnGpsDataReady(const std::function<void(GpsData&)> &newOnGpsDataReady);

private:
    uint32_t m_last_gps_shot;
    uint32_t m_gps_period = 5000;
    uint32_t m_min_distance = 20;

    std::function<void(GpsData&)> m_onGpsDataReady;
};