#pragma once

#include <stdint.h>
#include <JaaleeData.h>
#include <GpsData.h>
#include <M52PASData.h>

class DataHandler
{
public:
    static void handleJaaleeTemperatureData(const JaaleeData &data);
    static void handleM52PASTemperatureData(const M52PASData &data);
    static void handleGpsData(const GpsData &gpsData);

private:
};
