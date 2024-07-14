#include "GpsManagement.h"

void GpsManagement::loop(const uint32_t uptime_ms)
{
    switch (state)
    {
    case GpsManagementState::OFF:
        if (req_pwr_on)
        {
            turn_on_gps();
            state = GpsManagementState::STARTING;
        }
        break;
    case GpsManagementState::STARTING:
        state = GpsManagementState::ON;
        break;
    case GpsManagementState::ON:
        if (!req_pwr_on)
        {
            state = GpsManagementState::STOPPING;
        }
        else if ((measure_interval > 0u) && ((uptime_ms - last_shot) >= measure_interval))
        {
            last_shot = uptime_ms;
            read_gps();
        }
        break;
    case GpsManagementState::STOPPING:
        turn_off_gps();
        state = GpsManagementState::OFF;
        break;
    }
}

void GpsManagement::power_on(const bool req_state)
{
    req_pwr_on = req_state;
}

void GpsManagement::set_config(const QueueHandle_t out_queue, const uint32_t interval)
{
    out_queue_handle = out_queue;
    measure_interval = interval;
}

void GpsManagement::read_gps()
{
}

void GpsManagement::turn_on_gps()
{
}

void GpsManagement::turn_off_gps()
{
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
    double speed_knots = getNextSubstring(raw, ',', &iterator).toDouble();
    m_speed = speed_knots * 1.852000;

    (void)getNextSubstring(raw, ',', &iterator);
    (void)getNextSubstring(raw, ',', &iterator);
    m_precision = getNextSubstring(raw, ',', &iterator).toDouble();
}
