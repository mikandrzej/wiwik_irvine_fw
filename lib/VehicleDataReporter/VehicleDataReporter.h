#pragma once

#include <stdint.h>

class VehicleDataReporter
{
public:
    VehicleDataReporter() {};

    void init();
    void loop(const uint32_t timestamp);

private:
    bool isMoving();
    void report();

    float getSpeed();

    uint32_t lastReportTimestamp;
};