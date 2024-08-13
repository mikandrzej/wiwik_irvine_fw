#include "BluetoothConfiguration.h"
#include <string.h>
#include "ConfigurationHelpers.h"

bool BluetoothConfiguration::setMaxInterval(const uint32_t interval)
{
    devices[0].minInterval = interval;
    return true;
}

bool BluetoothConfiguration::setDeviceAddress(const char *const value)
{
    return ConfigurationHelpers::copyMacAddress(value, devices[0u].macAddress);
}
