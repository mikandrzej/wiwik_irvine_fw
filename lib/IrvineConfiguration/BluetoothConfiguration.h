#pragma once

#include <stdint.h>

#define MAX_BLUETOOTH_DEVICES 10u

enum class BluetoothDeviceType
{
    NONE,
    JAALEE_SENSOR,
    ALCOHOL_SENSOR,
    // Add more types as needed
};

struct BluetoothDevice
{
    BluetoothDeviceType type = BluetoothDeviceType::NONE;
    uint8_t macAddress[6u] = {0u};
    uint32_t maxInterval = 0u;
    float maxTemperatureChange = 0.0f;
    float maxHumidityChange = 0.0f;
};

class BluetoothConfiguration
{
public:
    BluetoothDevice devices[MAX_BLUETOOTH_DEVICES];
    bool deviceManagementEnable = false;

    bool setMaxInterval(uint32_t interval);
    bool setDeviceAddress(const char *const value);
};
