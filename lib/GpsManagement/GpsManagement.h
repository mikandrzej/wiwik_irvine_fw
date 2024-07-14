#pragma once

#include <stdint.h>
#include <Arduino.h>
#include "freertos/queue.h"

struct GpsData
{
    GpsData(String &raw);
    double m_speed;
    double m_latitude;
    double m_longitude;
    double m_altitude;
    double m_precision;
    int m_satellites;
    String m_timestamp;
    String m_raw;
};

enum class GpsManagementState
{
    OFF,
    STARTING,
    ON,
    STOPPING
};

class GpsManagement
{
public:
    void loop(const uint32_t uptime_ms);
    void power_on(const bool req_state);
    void set_config(const QueueHandle_t out_queue, const uint32_t interval);

private:
    void read_gps();
    void turn_on_gps();
    void turn_off_gps();

private:
    GpsManagementState state = GpsManagementState::OFF;
    bool req_pwr_on = false;
    uint32_t measure_interval = 0u;
    QueueHandle_t out_queue_handle;
    uint32_t last_shot;
};