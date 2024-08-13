#pragma once

#include "AbstractBluetoothBleDevice.h"
#include "M52PASData.h"

class BluetoothM52PASTempSensor : public AbstractBluetoothBleDevice
{
public:
    BluetoothM52PASTempSensor(const uint16_t configIndex);

    void parseAdvertisedData(const uint8_t *const data, const uint16_t len, int16_t rssi) override;

private:
    void notifyNeeded();

    bool firstRead = true;
    uint32_t lastShotTimestamp = 0u;
    float lastTemperature;
    float lastHumidity;
    float lastBattery;
};