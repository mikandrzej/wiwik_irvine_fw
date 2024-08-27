#include "VehicleDataReporter.h"

#include "IrvineConfiguration.h"

#include <GpsController.h>
#include <CanQueries/UdsVehicleSpeedQuery.h>
#include <CanQueries/UdsVehicleEngineSpeedQuery.h>
#include <CanQueries/UdsVehicleFeulLevelQuery.h>

#include <DataHandler.h>
#include <Device.h>

VehicleDataReporter vehicleDataReporter;

void VehicleDataReporter::init()
{
}

void VehicleDataReporter::loop()
{
    uint32_t interval;
    uint32_t timestamp = millis();

    checkMoving();

    if (isMoving)
        interval = irvineConfiguration.vehicle.movementLogInterval;
    else
        interval = irvineConfiguration.vehicle.stopLogInterval;

    if ((timestamp - lastReportTimestamp) >= interval)
    {
        report();
        lastReportTimestamp = timestamp;
    }
}

String VehicleDataReporter::logData()
{
    return savedlogData;
}

String VehicleDataReporter::logItem()
{
    return "vehicle";
}

String VehicleDataReporter::logMqttData()
{
    return savedlogData;
}

void VehicleDataReporter::checkMoving()
{
    switch (irvineConfiguration.vehicle.movementDetectionSource)
    {
    case VehicleMovementDetectionSource::ACCELEROMETER:
        isMoving = 0u;
        break;
    case VehicleMovementDetectionSource::GPS:
        isMoving = gpsController.isMoving() ? 1u : 0u;
        break;
    case VehicleMovementDetectionSource::CAN:
        isMoving = 0u;
        break;
    }
}

void VehicleDataReporter::report()
{
    char reportData[500];

    uint64_t unixTimestamp = device.getUnixTimestamp();
    if (unixTimestamp)
    {
        obtainData();

        size_t len = sprintf(reportData,
                             R"({"t":%llu,"spd":%0.1f,"mov":%d,"ign":%d,"fue":%.2f,"gps":%d)",
                             device.getUnixTimestamp(),
                             speed,
                             isMoving,
                             isIgnitionOn,
                             fuelLevel,
                             gpsDataValid ? 1u : 0u);
        if (gpsDataValid)
        {
            len += sprintf(&reportData[len],
                           R"(,"gt":%llu,"lng":%.5f,"lat":%.5f,"alt":%.1f,"spd":%.2f,"sat":%d)",
                           gpsData.gpsUnixTimestamp,
                           gpsData.longitude,
                           gpsData.latitude,
                           gpsData.altitude,
                           gpsData.speed,
                           gpsData.satellites);
        }
        len += sprintf(&reportData[len], R"(})");

        savedlogData = String(reportData);
        DataHandler::handleData(*this);
    }
}

void VehicleDataReporter::obtainData()
{
    speed = getSpeed();
    engineSpeed = udsVehicleEngineSpeedQuery.getEngineSpeed();
    fuelLevel = udsVehicleFuelLevelQuery.getFuelLevel();
    gpsData = gpsController.getGpsData(&gpsDataValid);

    switch (irvineConfiguration.vehicle.ignitionSource)
    {
    case VehicleIgnitionSource::CAN:
        isIgnitionOn = engineSpeed > 0u ? 1u : 0u;
        break;
    case VehicleIgnitionSource::VOLTAGE:
        isIgnitionOn = 1u;
        break;
    }
}

float VehicleDataReporter::getSpeed()
{
    bool valid;
    float speed;
    switch (irvineConfiguration.vehicle.speedSource)
    {
    case VehicleSpeedSource::CAN:
        speed = udsVehicleSpeedQuery.getSpeed(&valid);
        if (valid)
            return speed;
        else
            return gpsData.speed;
    case VehicleSpeedSource::GPS:
        break;
    }
    return 0.0f;
}
