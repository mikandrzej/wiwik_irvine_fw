#pragma once

#include <stdint.h>

class GpsData
{
public:
    GpsData();
    GpsData(uint8_t mode,
            uint8_t satellites,
            double latitude,
            double longitude,
            double altitude,
            double speed,
            uint64_t gpsUnixTimestamp,
            uint64_t unixTimestamp);
    uint8_t mode = 0u;
    uint8_t satellites = 0u;
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0;
    double speed = 0.0;
    uint64_t gpsUnixTimestamp = 0u;
    uint64_t unixTimestamp = 0u;
    bool valid = false;

};