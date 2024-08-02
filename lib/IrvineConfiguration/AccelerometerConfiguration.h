#pragma once

class AccelerometerConfiguration
{
public:
    uint32_t movementThreshold = 0u;
    uint32_t movementStopDelay = 0u;
    bool logEnable = false;
    uint32_t crashDetectionThreshold = 0u;
    bool towDetection = false;
    // Add more parameters as needed
};
