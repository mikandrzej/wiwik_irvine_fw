#pragma once

#include <stdint.h>

typedef struct
{
    uint8_t mode;
    uint8_t satellites;
    double latitude;
    double longitude;
    double altitude;
    double speed;
    uint64_t unixTimestamp;
} GpsData;