
#include "Gps.h"

#define TINY_GSM_MODEM_SIM7600 // A7670's AT instruction is compatible with SIM7600
#include <TinyGsmClient.h>
extern TinyGsm modem;

void Gps::loop()
{
    uint32_t t = millis();
    uint32_t diff;
    
    diff = t - m_last_gps_shot;
    if (diff > m_gps_period)
    {
        m_last_gps_shot = t;

        String raw_gps = modem.getGPSraw();

        if(raw_gps.length() > 20)
        {
            Serial.println("Gps data: " + raw_gps);
            if(m_onGpsDataReady)
            {
                GpsData gpsData = {.raw = raw_gps};
                m_onGpsDataReady(gpsData);
            }
        }

    }
}

void Gps::setGpsMinDistance(const uint32_t meters)
{
    m_min_distance = meters;
}

void Gps::setGpsPeriod(const uint32_t period)
{
    m_gps_period = period;
}

void Gps::setOnGpsDataReady(const std::function<void(GpsData&)> &newOnGpsDataReady)
{
    m_onGpsDataReady = newOnGpsDataReady;
}