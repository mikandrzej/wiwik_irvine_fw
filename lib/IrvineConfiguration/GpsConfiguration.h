#pragma once

#include <stdint.h>

class GpsConfiguration
{
public:
    uint32_t minimumDistance = 0u;
    uint32_t maxInterval = 0u;
    uint32_t sleepAfterIgnitionOffTimeout = 0u;
    bool freezePositionDuringStop = false;
    uint32_t movementSpeedThreshold = 0u;
    uint32_t movementStopDelay = 0u;
    uint32_t movementLogInterval = 0u;
    uint32_t stopLogInterval = 0u;
    bool highPrecisionOnDemand = false;
    uint32_t highPrecisionOnDemandDuration = 0u;
    bool jammingDetection = false;
    // Add more parameters as needed
};
