#include "DeviceConfiguration.h"

bool DeviceConfiguration::setBatteryInterval(const uint32_t value)
{
    batteryInterval = value;
    return true;
}

bool DeviceConfiguration::setLogSeverity(const LogSeverity value)
{
    logSeverity = value;
    return false;
}
