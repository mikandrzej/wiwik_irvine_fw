#pragma once

#include <stdint.h>

class DataHandler
{
public:
    static void handleJaaleeTemperatureData(const uint8_t bluetoothDeviceId, const float temperature, const float humidity, const float battery);
    static void handleGpsData(const float longitude, const float latitude, const float altitude, const float speed, const uint64_t unixTimestamp, const uint8_t satellites, const float precision);
};
