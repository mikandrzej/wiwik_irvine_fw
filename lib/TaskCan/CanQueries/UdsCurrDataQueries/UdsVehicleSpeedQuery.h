#pragma once

#include "../UdsCurrDataQuery.h"

#include <DataLoggable.h>

using UdsVehicleSpeedQueryCallback = std::function<void(const uint8_t)>;

class UdsVehicleSpeedQuery : public UdsCurrDataQuery, public DataLoggable
{
public:
    UdsVehicleSpeedQuery(uint32_t interval);

    bool parseReceivedValue(uint8_t dataLen, uint8_t *data);

    uint8_t getLastValue();

    String logData();
    String logItem();
    String logMqttData();

private:

    uint8_t valueReceived = false;
    uint8_t lastValue = 0u;
};