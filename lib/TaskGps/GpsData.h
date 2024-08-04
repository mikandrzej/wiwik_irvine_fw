#pragma once

#include <stdint.h>

struct GpsData
{
    uint8_t mode;
    uint8_t satellites;
    double latitude;
    double longitude;
    double altitude;
    double speed;
    uint64_t unixTimestamp;
};