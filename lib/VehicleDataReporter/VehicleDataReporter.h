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
    void checkMoving();
    void report();

    void obtainData();

    float getSpeed();

    uint32_t lastReportTimestamp;

    float speed;
    float fuelLevel;
    uint16_t engineSpeed;
    GpsData gpsData;
    bool gpsDataValid;
    uint8_t isMoving;
    uint8_t isIgnitionOn;
    String savedlogData = "";
};

extern VehicleDataReporter vehicleDataReporter;