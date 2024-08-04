#pragma once

#include <stdint.h>
#include <GpsData.h>

class DataHandler
{
public:
    static void handleJaaleeTemperatureData(const uint8_t bluetoothDeviceId, const float temperature, const float humidity, const float battery, const int16_t rssi);
    static void handleGpsData(const GpsData &gpsData);

private:
};
