#include "VehicleDataReporter.h"

#include "IrvineConfiguration.h"

#include <GpsController.h>
#include <CanQueries/UdsVehicleSpeedQuery.h>
#include <CanQueries/UdsVehicleEngineSpeedQuery.h>
#include <CanQueries/UdsVehicleFeulLevelQuery.h>

#include <DataHandler.h>
#include <Device.h>
#include <Vehicle.h>

VehicleDataReporter vehicleDataReporter;

void VehicleDataReporter::init()
{
}

void VehicleDataReporter::loop()
{
    uint32_t interval;
    uint32_t timestamp = millis();

    bool valid;
    bool isMoving = vehicle.isMoving(&valid);

    if (!valid)
        isMoving = true;

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

void VehicleDataReporter::report()
{
    char reportData[500];
    bool valid;

    uint64_t unixTimestamp = device.getUnixTimestamp();
    if (unixTimestamp)
    {
        size_t len = sprintf(reportData, R"({"tim":%llu)", unixTimestamp);

        float speed = vehicle.getSpeed(&valid);
        if (valid)
            len += sprintf(&reportData[len], R"(,"vel":%.2f)", speed);

        uint16_t engineSpeed = udsVehicleEngineSpeedQuery.getEngineSpeed(&valid);
        if (valid)
            len += sprintf(&reportData[len], R"(,"eng":%u)", engineSpeed);

        float fuelLevel = udsVehicleFuelLevelQuery.getFuelLevel(&valid);
        if (valid)
            len += sprintf(&reportData[len], R"(,"fue":%.2f)", fuelLevel);

        float voltage = vehicle.getVccVoltage(&valid);
        if (valid)
            len += sprintf(&reportData[len], R"(,"bat":%.2f)", voltage);

        bool ingintionOn = vehicle.isIgnitionOn(&valid);
        if (valid)
            len += sprintf(&reportData[len], R"(,"ign":%d)", ingintionOn ? 1 : 0);

        GpsData gpsData = gpsController.getGpsData(&valid);
        if (valid)
        {
            len += sprintf(&reportData[len],
                           R"(,"gps":{"t":%llu,"lng":%.5f,"lat":%.5f,"alt":%.1f,"vel":%.2f,"sat":%d})",
                           gpsData.gpsUnixTimestamp,
                           gpsData.longitude,
                           gpsData.latitude,
                           gpsData.altitude,
                           gpsData.speed,
                           gpsData.satellites);
        }

        len += sprintf(&reportData[len], "}");

        savedlogData = String(reportData);
        DataHandler::handleData(*this);
    }
}
