#pragma once

#include <stdint.h>
#include <Logger.h>


class DeviceConfiguration
{
public:
    uint32_t batteryInterval = 0u;
    uint32_t ignitionVoltageThreshold = 0u;
    uint32_t ignitionOffDelay = 0u;
    uint32_t movementLogInterval = 0u;
    uint32_t stopLogInterval = 0u;
    bool powerOutageDetection = false; // module uninstall
    bool privateBusinessRide = false;  // reporting off
    char deviceId[50u] = {0u};
    LogSeverity logSeverity = LogSeverity::ERROR;
    float batteryCalibrationScale = 1.0f;
    char password[20] = {0u};
};
