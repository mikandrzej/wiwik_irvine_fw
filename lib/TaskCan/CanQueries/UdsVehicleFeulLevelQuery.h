#pragma once

#include "UdsCurrDataQuery.h"

#include <DataLoggable.h>

class UdsVehicleFuelLevelQuery : public UdsCurrDataQuery, public DataLoggable
{
public:
    UdsVehicleFuelLevelQuery() : UdsCurrDataQuery(0x2Fu) {}
    void loop() override;

    float getFuelLevel(bool *valid = nullptr);

    String logData();
    String logItem();
    String logMqttData();

private:
    bool parseCurrDataResponse(uint8_t dataLen, uint8_t *data) override;

    uint32_t getIntervalValue() override;
    uint32_t getTimeoutValue() override;

    bool valueReceived = false;
    float lastValue = 0u;
};

extern UdsVehicleFuelLevelQuery udsVehicleFuelLevelQuery;