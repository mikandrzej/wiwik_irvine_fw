#pragma once

#include <Arduino.h>

typedef struct
{
    char mqtt_server_address[50u];
    uint16_t mqtt_server_port;
    char mqtt_server_login[30u];
    char mqtt_server_password[30u];
    uint32_t gps_report_interval;
    uint32_t temperature_report_interval;
    uint32_t battery_report_interval;
    uint32_t network_report_interval;
    char jaale_sensor_address[18u];
    uint8_t debug_mode;

    uint32_t crc;
} __attribute__((packed)) config_data_s;

class Configuration
{
public:
    void cyclic();
    void initSource();
    void readConfig();

    void defaultConfig();
    void printConfig();

    String getMqttServerAddress();
    uint16_t getMqttServerPort();
    String getMqttServerLogin();
    String getMqttServerPassword();
    uint32_t getGpsReportInterval();
    uint32_t getTemperatureReportInterval();
    uint32_t getBatteryReportInterval();
    uint32_t getNetworkReportInterval();
    String getJaaleSensorAddress();
    uint8_t getDebugMode();

    void setMqttServerAddress(String &address);
    void setMqttServerPort(uint16_t port);
    void setMqttServerLogin(String &login);
    void setMqttServerPassword(String &password);
    void setGpsReportInterval(const uint32_t interval);
    void setTemperatureReportInterval(const uint32_t interval);
    void setBatteryReportInterval(const uint32_t interval);
    void setNetworkReportInterval(const uint32_t interval);
    void setJaaleSensorAddress(String &address);
    void setDebugMode(const uint8_t mode);

private:
    bool saveConfig();
    uint32_t calculateCrc();

private:
    config_data_s m_currentConfig;
    bool m_configChanged;
    bool m_configReady;
};

extern Configuration configuration;
