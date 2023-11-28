
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
                GpsData gpsData(raw_gps);
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

static String getNextSubstring(String &input, char separator, int *iterator)
{
    int start = *iterator;
    int end = *iterator;

    
    if(input[end] == separator)
    {
        (*iterator)++;
        return input.substring(start, end);
    }
    while(end < input.length())
    {
        if(input[end] == separator)
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

    m_latitude = getNextSubstring(raw, ',', &iterator).toDouble();
    char ns = getNextSubstring(raw, ',', &iterator)[0];
    if (ns != 'N')
        m_latitude *= -1.0;

    m_longitude = getNextSubstring(raw, ',', &iterator).toDouble();
    char ew = getNextSubstring(raw, ',', &iterator)[0];
    if (ns != 'E')
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


    Serial.printf("sat: %d\r\n", m_satellites);
    Serial.printf("lat: %.6f\r\n", m_latitude);
    Serial.printf("lon: %.6f\r\n", m_longitude);
    Serial.printf("alt: %.6f\r\n", m_altitude);
    Serial.printf("spd: %.2f\r\n", m_speed);
    Serial.printf("precision: %.2f\r\n", m_precision);
    Serial.print("timestamp: " + m_timestamp);
}
