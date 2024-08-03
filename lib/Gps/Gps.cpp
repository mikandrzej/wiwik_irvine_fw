
#include "Gps.h"
#include <IrvineConfiguration.h>
// #include <cmath>
#include <Logger.h>
#include <ModemManagement.h>
#include <DataHandler.h>

#include <Device.h>

#include <TimeLib.h>

const char MODULE[] = "GPS";

Gps gps;

void Gps::loop()
{
    if (xSemaphoreTake(modemSemaphore, (TickType_t)portMAX_DELAY) == pdTRUE)
    {
        String raw_gps = modem.getGPSraw();
        xSemaphoreGive(modemSemaphore);

        if (raw_gps.length() > 10)
        {
            logger.logPrintF(LogSeverity::DEBUG, MODULE, "Received GPS data: %s", raw_gps.c_str());

            GpsData gpsData(raw_gps);

            device.updateGpsTime(gpsData.m_unixTimestamp);

            if (updateRequired(gpsData.m_latitude, gpsData.m_longitude))
            {
                DataHandler::handleGpsData(gpsData.m_longitude,
                                           gpsData.m_latitude,
                                           gpsData.m_altitude,
                                           gpsData.m_speed,
                                           gpsData.m_unixTimestamp,
                                           gpsData.m_satellites,
                                           gpsData.m_precision);
            }
            else
            {
                logger.logPrintF(LogSeverity::DEBUG, MODULE, "GPS data update not required");
            }
        }
    }
}

bool Gps::updateRequired(double latitude, double longitude)
{
    bool result = false;
    constexpr double m_per_unit = 111.2;
    if (m_last_latitude > 360.0)
    {
        result = true;
    }
    else if ((millis() - m_last_gps_try) > irvineConfiguration.gps.maxInterval)
    {
        result = true;
    }
    else
    {
        auto distance_lat = (latitude - m_last_latitude) * m_per_unit;
        auto distance_long = (longitude - m_last_longitude) * m_per_unit;
        auto distance_sqr = distance_lat * distance_long;
        if (distance_sqr < 0.0)
            distance_sqr *= -1.0;

        // auto distance = sqrt(distance_sqr);
        static auto min_distance_sqr = (double)irvineConfiguration.gps.minimumDistance * (double)irvineConfiguration.gps.minimumDistance;

        if (distance_sqr > min_distance_sqr)
        {
            result = true;
        }
    }

    if (true == result)
    {
        m_last_gps_try = millis();
        m_last_latitude = latitude;
        m_last_longitude = longitude;
    }

    return result;
}

static String getNextSubstring(String &input, char separator, int *iterator)
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

GpsData::GpsData(String &raw)
{
    Serial.println("Parsing gps data...");
    m_raw = raw;
    int iterator = 0;
    m_mode = (uint8_t)getNextSubstring(raw, ',', &iterator).toInt();
    m_satellites = getNextSubstring(raw, ',', &iterator).toInt();

    (void)getNextSubstring(raw, ',', &iterator);
    (void)getNextSubstring(raw, ',', &iterator);
    (void)getNextSubstring(raw, ',', &iterator);

    // format 1: +CGNSSINFO: 3,12,,  ,  ,52.3953667,N,16.7443104 ,E,111223,213028.00,107.7,0.992,,3.18
    // format 2: +CGNSSINFO: 3,14,,00,01,5223.72163,N,01644.66370,E,111223,213524.00,93.5 ,4.551,,
    auto latitude = getNextSubstring(raw, ',', &iterator);
    char ns = getNextSubstring(raw, ',', &iterator)[0];
    auto longitude = getNextSubstring(raw, ',', &iterator);
    char ew = getNextSubstring(raw, ',', &iterator)[0];

    if (latitude[2] == '.')
    {
        m_latitude = latitude.toDouble();
        m_longitude = longitude.toDouble();
    }
    else
    {
        auto lat = latitude.toDouble();
        m_latitude = (int)lat / 100;
        lat -= m_latitude * 100;
        m_latitude += lat / 60.0;

        auto lng = longitude.toDouble();
        m_longitude = (int)lng / 100;
        lng -= m_longitude * 100;
        m_longitude += lng / 60.0;
    }

    if (ns != 'N')
        m_latitude *= -1.0;
    if (ew != 'E')
        m_longitude *= -1.0;

    String date = getNextSubstring(raw, ',', &iterator);
    String time = getNextSubstring(raw, ',', &iterator);

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

    m_unixTimestamp = makeTime(tm);

    m_altitude = getNextSubstring(raw, ',', &iterator).toDouble();
    double speed_knots = getNextSubstring(raw, ',', &iterator).toDouble();
    m_speed = speed_knots * 1.852000;

    m_precision = getNextSubstring(raw, ',', &iterator).toDouble();
    (void)getNextSubstring(raw, ',', &iterator);
    (void)getNextSubstring(raw, ',', &iterator);
}
