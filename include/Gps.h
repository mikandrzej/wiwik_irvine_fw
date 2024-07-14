#pragma once

#include <Arduino.h>


class Gps
{
public:
    void setup();
    void loop();
    void setGpsMinDistance(const uint32_t meters);
    void setOnGpsDataReady(const std::function<void(GpsData&)> &newOnGpsDataReady);


private:
    uint32_t m_last_gps_try;
    uint32_t m_last_gps_shot;
    uint32_t m_min_distance = 20;
    uint32_t m_max_time = 1 * 60 * 1000;
    double m_last_latitude = 999.0;
    double m_last_longitude = 999.0;

    std::function<void(GpsData&)> m_onGpsDataReady;
    bool updateRequired(double latitude, double longitude);
};