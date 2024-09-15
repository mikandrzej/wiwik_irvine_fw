#include "GpsObserver.h"
#include <Logger.h>

#include <ModemManagement.h>
#include <IrvineConfiguration.h>

#include <DataHandler.h>

const char MODULE[] = "GPS_OBS";

GpsObserver gpsObserver;

bool GpsObserver::begin()
{
    return true;
}

void GpsObserver::loop()
{
    GpsData gpsData;

    if (!modemGpsRxQueue)
        return;

    if (xQueueReceive(modemGpsRxQueue, &gpsData, 0))
    {
        handleNewGpsData(gpsData);
    }
}

bool GpsObserver::isMoving(bool *valid)
{
    if (valid)
        *valid = (millis() - lastShotTimestamp) < 10000;
    return moving;
}

GpsData &GpsObserver::getLastGpsData()
{
    return lastReceivedData;
}

void GpsObserver::handleNewGpsData(GpsData &gpsData)
{
    logger.logPrintF(LogSeverity::DEBUG, MODULE, "Gps data mode:%u, sat:%u, %f %f %f, speed:%f, timestamp: %llu",
                     gpsData.mode,
                     gpsData.satellites,
                     gpsData.latitude,
                     gpsData.longitude,
                     gpsData.altitude,
                     gpsData.speed,
                     gpsData.unixTimestamp);

    if (firstMeasure)
    {
        firstMeasure = false;
    }
    else
    {
        float distance = getDistanceFromLastShot(gpsData);
        logger.logPrintF(LogSeverity::DEBUG, MODULE, "GPS distance from last shot: %.1f", distance);
        if (distance >= (float)irvineConfiguration.gps.minimumDistance)
        {
            moving = true;
        }
        else
        {
            moving = false;
        }

        lastReceivedData = gpsData;
    }
}

float GpsObserver::getDistanceFromLastShot(const GpsData &gpsData)
{
    const double R = 6371000.0; // Promień Ziemi w metrach

    // Konwersja stopni na radiany
    double lat1Rad = degreesToRadians(lastReceivedData.latitude);
    double lon1Rad = degreesToRadians(lastReceivedData.longitude);
    double lat2Rad = degreesToRadians(gpsData.latitude);
    double lon2Rad = degreesToRadians(gpsData.longitude);

    // Różnica między współrzędnymi
    double dLat = lat2Rad - lat1Rad;
    double dLon = lon2Rad - lon1Rad;

    // Wzór Haversine
    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
               std::cos(lat1Rad) * std::cos(lat2Rad) *
                   std::sin(dLon / 2) * std::sin(dLon / 2);

    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

    double distance = R * c;

    return distance;
}

float GpsObserver::degreesToRadians(float degree)
{
    return degree * (M_PI / 180.0);
}