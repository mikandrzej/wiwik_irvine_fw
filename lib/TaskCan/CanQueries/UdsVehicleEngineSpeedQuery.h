#pragma once

#include "UdsCurrDataQuery.h"

#include <DataLoggable.h>

class UdsVehicleEngineSpeedQuery : public UdsCurrDataQuery, public DataLoggable
{
public:
    UdsVehicleEngineSpeedQuery() : UdsCurrDataQuery(0x0Cu) {}
    void loop() override;

    uint16_t getEngineSpeed(bool *valid = nullptr);

    String logData();
    String logItem();
    String logMqttData();

private:
    bool parseCurrDataResponse(uint8_t dataLen, uint8_t *data) override;

    uint32_t getIntervalValue() override;
    uint32_t getTimeoutValue() override;

    bool valueReceived = false;
    uint16_t lastValue = 0u;
};

extern UdsVehicleEngineSpeedQuery udsVehicleEngineSpeedQuery;