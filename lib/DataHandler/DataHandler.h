#pragma once

#include <stdint.h>
#include <JaaleeData.h>
#include <GpsData.h>

class DataHandler
{
public:
    static void handleJaaleeTemperatureData(const JaaleeData &data, const uint8_t bluetoothDeviceId);
    static void handleGpsData(const GpsData &gpsData);

private:
};
