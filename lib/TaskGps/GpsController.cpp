#include "GpsController.h"

#include <ModemManagement.h>
#include <DataHandler.h>
#include <Logger.h>
#include <IrvineConfiguration.h>

#include "EgTinyGsm.h"
#include <TimeLib.h>
#include <cmath>

const char MODULE[] = "GPS_CTRL";

GpsController gpsController;

void GpsController::loop()
{
    switch (state)
    {
    case GpsState::UNINITIALIZED:
        if (xSemaphoreTake(modemSemaphore, portMAX_DELAY) == pdTRUE)
        {
            logger.logPrintF(LogSeverity::INFO, MODULE, "GPS initialization start...");
            modem.sendAT("+CGNSSPWR=0");
            if (modem.waitResponse(1000) == 1)
            {
                logger.logPrintF(LogSeverity::DEBUG, MODULE, "GPS initialization 1st stage done");
                modem.sendAT("+CGNSSPWR=1");
                if (modem.waitResponse(1000) == 1)
                {
                    logger.logPrintF(LogSeverity::DEBUG, MODULE, "GPS initialization 2nd stage done");
                    if (modem.waitResponse(20000UL, "+CGNSSPWR: READY!") == 1)
                    {
                        logger.logPrintF(LogSeverity::DEBUG, MODULE, "GPS initialization 3rd stage done");
                        modem.sendAT("+CGNSSMODE=3");
                        if (modem.waitResponse(1000) == 1)
                        {
                            logger.logPrintF(LogSeverity::INFO, MODULE, "GPS initialization done");
                        }
                        else
                        {
                            logger.logPrintF(LogSeverity::ERROR, MODULE, "GPS initialization 4th stage error");
                        }
                    }
                    else
                    {
                        logger.logPrintF(LogSeverity::ERROR, MODULE, "GPS initialization 3rd stage error");
                    }
                }
                else
                {
                    logger.logPrintF(LogSeverity::ERROR, MODULE, "GPS initialization 2nd stage error");
                }
            }
            else
            {
                logger.logPrintF(LogSeverity::ERROR, MODULE, "GPS initialization 1st stage error");
            }

            xSemaphoreGive(modemSemaphore);

            state = GpsState::READY;
        }
        break;
    case GpsState::READY:
        if (xSemaphoreTake(modemSemaphore, portMAX_DELAY) == pdTRUE)
        {
            String rawGpsData = modem.getGPSraw();
            xSemaphoreGive(modemSemaphore);

            parseGpsData(rawGpsData);
        }
        break;
    }
}

void GpsController::parseGpsData(const String &data)
{
    if (data.length() < 20)
        return;
    int iterator = 0;
    uint8_t mode = (uint8_t)getNextSubstring(data, ',', &iterator).toInt();
    uint8_t satellites = (uint8_t)getNextSubstring(data, ',', &iterator).toInt();

    (void)getNextSubstring(data, ',', &iterator);
    (void)getNextSubstring(data, ',', &iterator);
    (void)getNextSubstring(data, ',', &iterator);

    // format 1: +CGNSSINFO: 3,12,,  ,  ,52.3953667,N,16.7443104 ,E,111223,213028.00,107.7,0.992,,3.18
    // format 2: +CGNSSINFO: 3,14,,00,01,5223.72163,N,01644.66370,E,111223,213524.00,93.5 ,4.551,,

    auto latitudeStr = getNextSubstring(data, ',', &iterator);
    char ns = getNextSubstring(data, ',', &iterator)[0];
    auto longitudeStr = getNextSubstring(data, ',', &iterator);
    char ew = getNextSubstring(data, ',', &iterator)[0];

    double latitude;
    double longitude;

    if (latitudeStr[2] == '.')
    {
        latitude = latitudeStr.toDouble();
        longitude = longitudeStr.toDouble();
    }
    else
    {
        auto lat = latitudeStr.toDouble();
        latitude = (int)lat / 100;
        lat -= latitude * 100;
        latitude += lat / 60.0;

        auto lng = longitudeStr.toDouble();
        longitude = (int)lng / 100;
        lng -= longitude * 100;
        longitude += lng / 60.0;
    }

    if (ns != 'N')
        latitude *= -1.0;
    if (ew != 'E')
        longitude *= -1.0;

    String date = getNextSubstring(data, ',', &iterator);
    String time = getNextSubstring(data, ',', &iterator);

    tmElements_t tm;
    int day, month, year, hour, minute, second;
    sscanf(date.c_str(), "%2d%2d%2d", &day, &month, &year);
    sscanf(time.c_str(), "%2d%2d%2d", &hour, &minute, &second);

    tm.Day = day;
    tm.Month = month;
    tm.Year = CalendarYrToTm(2000 + year); // Dodajemy 2000, ponieważ rok podawany jest jako YY
    tm.Hour = hour;
    tm.Minute = minute;
    tm.Second = second;

    uint64_t unixTimestamp = makeTime(tm);

    double altitude = getNextSubstring(data, ',', &iterator).toDouble();
    double speed_knots = getNextSubstring(data, ',', &iterator).toDouble();
    double speed = speed_knots * 1.852000;

    (void)getNextSubstring(data, ',', &iterator);
    (void)getNextSubstring(data, ',', &iterator);

    handleGpsData({mode, satellites, latitude, longitude, altitude, speed, unixTimestamp});
}

void GpsController::handleGpsData(const GpsData &gpsData)
{
    logger.logPrintF(LogSeverity::DEBUG, MODULE, "Gps data mode:%u, sat:%u, %f %f %f, speed:%f, timestamp: %llu",
                     gpsData.mode,
                     gpsData.satellites,
                     gpsData.latitude,
                     gpsData.longitude,
                     gpsData.altitude,
                     gpsData.speed,
                     gpsData.unixTimestamp);

    if (firstShot)
    {
        firstShot = false;
        publishNewData(gpsData);
    }
    else
    {
        const uint32_t diff = millis() - lastShotTimestamp;
        if (diff >= irvineConfiguration.gps.maxInterval)
        {
            publishNewData(gpsData);
        }
        else
        {
            float distance = getDistanceFromLastShot(gpsData);
            if (distance >= irvineConfiguration.gps.minimumDistance)
            {
                publishNewData(gpsData);
            }
        }
    }
}

void GpsController::publishNewData(const GpsData &gpsData)
{
    lastPublishedData = gpsData;
    lastShotTimestamp = millis();
    DataHandler::handleGpsData(gpsData);
}

String GpsController::getNextSubstring(const String &input, char separator, int *iterator)
{
    int start = *iterator;
    int end = *iterator;

    if (input[end] == separator)
    {
        (*iterator)++;
        return input.substring(start, end);
    }
    while (end < input.length())
    {
        if (input[end] == separator)
        {
            *iterator = end + 1;
            return input.substring(start, end);
        }
        end++;
    }
    *iterator = end;
    return input.substring(start, end);
}

float GpsController::degreesToRadians(float degree)
{
    return degree * (M_PI / 180.0);
}

float GpsController::getDistanceFromLastShot(const GpsData &gpsData)
{
    const double R = 6371000.0; // Promień Ziemi w metrach

    // Konwersja stopni na radiany
    double lat1Rad = degreesToRadians(lastPublishedData.latitude);
    double lon1Rad = degreesToRadians(lastPublishedData.longitude);
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