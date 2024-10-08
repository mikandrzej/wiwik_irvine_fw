#pragma once

#include <BluetoothConfiguration.h>

class AbstractBluetoothBleDevice
{
public:
    AbstractBluetoothBleDevice(uint8_t configIndex);

    virtual void parseAdvertisedData(const uint8_t *const data, const uint16_t len, int16_t rssi) = 0;

protected:
    const uint8_t configIndex;
};