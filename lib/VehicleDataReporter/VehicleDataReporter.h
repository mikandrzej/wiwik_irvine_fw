#pragma once

#include <stdint.h>
#include <GpsData.h>
#include <DataLoggable.h>

class VehicleDataReporter : DataLoggable
{
public:
    VehicleDataReporter() {};

    void init();
    void loop();

    String logData() override;
    String logItem() override;
    String logMqttData() override;

private:
    void report();

    uint32_t lastReportTimestamp = 0u;

    String savedlogData = "";
};

extern VehicleDataReporter vehicleDataReporter;