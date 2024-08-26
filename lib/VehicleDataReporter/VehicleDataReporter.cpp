#include "VehicleDataReporter.h"

#include "IrvineConfiguration.h"

#include <GpsController.h>
#include <CanQueries/UdsVehicleSpeedQuery.h>

void VehicleDataReporter::init()
{
}

void VehicleDataReporter::loop(const uint32_t timestamp)
{
    uint32_t interval;

    if (isMoving())
        interval = irvineConfiguration.vehicle.movementLogInterval;
    else
        interval = irvineConfiguration.vehicle.stopLogInterval;

    if ((lastReportTimestamp - timestamp) >= interval)
    {
        report();
        lastReportTimestamp = timestamp;
    }
}

bool VehicleDataReporter::isMoving()
{
    switch (irvineConfiguration.vehicle.movementDetectionSource)
    {
    case VehicleMovementDetectionSource::ACCELEROMETER:
        break;
    case VehicleMovementDetectionSource::GPS:
        return gpsController.isMoving();
    case VehicleMovementDetectionSource::CAN:
        break;
    }
}
void VehicleDataReporter::report()
{
    float speed = getSpeed();
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
        break;
    case VehicleSpeedSource::GPS:
        break;
    }
    return 0.0f;
}
