#pragma once

#include "UdsCurrDataQuery.h"

#include <DataLoggable.h>

using UdsVehicleSpeedQueryCallback = std::function<void(const uint8_t)>;

class UdsVehicleSpeedQuery : public UdsCurrDataQuery, public DataLoggable
{
public:
    UdsVehicleSpeedQuery() : UdsCurrDataQuery(0x0Du) {}
    void loop() override;

    uint8_t getSpeed(bool *valid = nullptr);

    String logData();
    String logItem();
    String logMqttData();

private:
    bool parseCurrDataResponse(uint8_t dataLen, uint8_t *data) override;

    uint32_t getIntervalValue() override;
    uint32_t getTimeoutValue() override;

    uint8_t valueReceived = false;
    uint8_t lastValue = 0u;
};

extern UdsVehicleSpeedQuery udsVehicleSpeedQuery;