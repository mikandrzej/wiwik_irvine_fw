#pragma once

#include <stdint.h>

typedef struct
{
    float temperature;
    float humidity;
    float battery;
    int16_t rssi;
} JaaleeData;