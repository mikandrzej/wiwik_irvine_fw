#pragma once

#include <Arduino.h>

class GpsData
{
public:
    GpsData(String &raw);
    double m_speed;
    double m_latitude;
    double m_longitude;
    double m_altitude;
    double m_precision;
    int m_satellites;
    uint8_t m_mode; // 2 - 2D, 3 - 3D
    uint64_t m_unixTimestamp;
    String m_raw;
};

class Gps
{
public:
    void loop();

private:
    uint32_t m_last_gps_try;
    uint32_t m_last_gps_shot;
    double m_last_latitude = 999.0;
    double m_last_longitude = 999.0;

    bool updateRequired(double latitude, double longitude);
};