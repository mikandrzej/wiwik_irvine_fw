#include "AbstractBluetoothBleDevice.h"
#include <Arduino.h>
#include <IrvineConfiguration.h>
#include <Logger.h>

const char MODULE[] = "BLE_DEVICE";

AbstractBluetoothBleDevice::AbstractBluetoothBleDevice(const uint8_t configIndex) : configIndex(configIndex)
{
}

