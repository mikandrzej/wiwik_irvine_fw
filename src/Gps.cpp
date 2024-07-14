
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

                // if (!updateRequired(gpsData.m_latitude, gpsData.m_longitude))
                // {
                //     Serial.println("Update not required");
                // }
                // else
                // {
                    m_onGpsDataReady(gpsData);
                // }
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