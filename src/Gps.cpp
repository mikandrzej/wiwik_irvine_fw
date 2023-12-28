
#include "Gps.h"
#include "Configuration.h"
// #include <cmath>

#define TINY_GSM_MODEM_SIM7600 // A7670's AT instruction is compatible with SIM7600
#include <TinyGsmClient.h>
extern TinyGsm modem;

void Gps::loop()
{
    uint32_t t = millis();
    uint32_t diff;

    diff = t - m_last_gps_try;
    if (diff > configuration.getGpsReportInterval())
    {
        m_last_gps_try = t;

        String raw_gps = modem.getGPSraw();

        if (raw_gps.length() > 20)
        {
            Serial.println("Gps data: " + raw_gps);
            if (m_onGpsDataReady)
            {
                GpsData gpsData(raw_gps);

                if (!updateRequired(gpsData.m_latitude, gpsData.m_longitude))
                {
                    Serial.println("Update not required");
                }
                else
                {
                    m_onGpsDataReady(gpsData);
                }
            }
        }
    }
}

void Gps::setGpsMinDistance(const uint32_t meters)
{
    m_min_distance = meters;
}

void Gps::setOnGpsDataReady(const std::function<void(GpsData &)> &newOnGpsDataReady)
{
    m_onGpsDataReady = newOnGpsDataReady;
}

bool Gps::updateRequired(double latitude, double longitude)
{
    bool result = false;
    constexpr double m_per_unit = 111.2;
    if (m_last_latitude > 360.0)
    {
        result = true;
    }
    else if ((millis() - m_last_gps_try) > m_max_time)
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
        auto min_distance_sqr = (double)(m_min_distance * m_min_distance);

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
    (void)getNextSubstring(raw, ',', &iterator).toInt();
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
    String day = date.substring(0, 2);
    String month = date.substring(2, 4);
    String year = "20" + date.substring(4, 6);
    String time = getNextSubstring(raw, ',', &iterator);
    String hour = time.substring(0, 2);
    String minute = time.substring(2, 4);
    String second = time.substring(4, 6);

    m_timestamp = year + "-" + month + "-" + day + " " + hour + ":" + minute + ":" + second + " GMT+0";

    m_altitude = getNextSubstring(raw, ',', &iterator).toDouble();
    m_speed = getNextSubstring(raw, ',', &iterator).toDouble();

    (void)getNextSubstring(raw, ',', &iterator);
    (void)getNextSubstring(raw, ',', &iterator);
    m_precision = getNextSubstring(raw, ',', &iterator).toDouble();
}
