#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include "File.h"
#include "Configuration.h"
#include <EEPROM.h>
#include <esp_crc.h>

#include <Preferences.h>

#define EEPROM_SIZE sizeof(eeprom_data_s)

Preferences prefs;

void Configuration::initSource()
{
    // EEPROM.begin(EEPROM_SIZE);
    prefs.begin("configuration"); // namespace
}

void Configuration::printConfig()
{
    Serial.printf("Configuration:\r\n");
    Serial.printf("MQTT server: %s\r\n", m_currentConfig.mqtt_server_address);
    Serial.printf("MQTT port: %d\r\n", m_currentConfig.mqtt_server_port);
    Serial.printf("MQTT username: %s\r\n", m_currentConfig.mqtt_server_login);
    Serial.printf("MQTT password: %s\r\n", m_currentConfig.mqtt_server_password);
    Serial.printf("Temperature report interval: %u\r\n", m_currentConfig.temperature_report_interval);
    Serial.printf("GPS report interval: %u\r\n", m_currentConfig.gps_report_interval);
    Serial.printf("Battery report interval: %u\r\n", m_currentConfig.battery_report_interval);
    Serial.printf("Jaale sensor address: %s\r\n", m_currentConfig.jaale_sensor_address);
}

void Configuration::readConfig()
{
    m_configReady = false;

    int retries = 3;
    do
    {
        // EEPROM.readBytes(0u, &m_currentConfig, sizeof(m_currentConfig));

        prefs.getBytes("configuration", &m_currentConfig, sizeof(m_currentConfig));
        uint32_t crc = calculateCrc();
        if (crc == m_currentConfig.crc)
        {
            Serial.println("Configration read succesfully");
            Serial.printf("Configration CRC: 0x%08x\r\n", m_currentConfig.crc);
            m_configReady = true;
            break;
        }

        Serial.println("Error during configuration read, retry...");
        retries--;
    } while (retries > 0);

    if (!m_configReady)
    {
        Serial.println("Restoring default configuration.");
        defaultConfig();
    }

    printConfig();
}

String Configuration::getMqttServerAddress()
{
    if (m_configReady)
        return String(m_currentConfig.mqtt_server_address);
    return String();
}

uint16_t Configuration::getMqttServerPort()
{
    if (m_configReady)
        return m_currentConfig.mqtt_server_port;
    return 0;
}

String Configuration::getMqttServerLogin()
{
    if (m_configReady)
        return String(m_currentConfig.mqtt_server_login);
    return String();
}

String Configuration::getMqttServerPassword()
{
    if (m_configReady)
        return String(m_currentConfig.mqtt_server_password);
    return String();
}

uint32_t Configuration::getGpsReportInterval()
{
    if (m_configReady)
        return m_currentConfig.gps_report_interval;
    return 0;
}

uint32_t Configuration::getTemperatureReportInterval()
{
    if (m_configReady)
        return m_currentConfig.temperature_report_interval;
    return 0;
}

uint32_t Configuration::getBatteryReportInterval()
{
    if (m_configReady)
        return m_currentConfig.battery_report_interval;
    return 0;
}

uint32_t Configuration::getNetworkReportInterval()
{
    if (m_configReady)
        return m_currentConfig.network_report_interval;
    return 0;
}

String Configuration::getJaaleSensorAddress()
{
    if (m_configReady)
        return String(m_currentConfig.jaale_sensor_address);
    return String();
}

uint8_t Configuration::getDebugMode()
{
    if (m_configReady)
        return m_currentConfig.debug_mode;
    return 0;
}

bool Configuration::saveConfig()
{

    int retries = 3;
    do
    {
        eeprom_data_s temp_config;
        prefs.getBytes("configuration", &temp_config, sizeof(temp_config));
        if (0 == memcmp(&temp_config, &m_currentConfig, sizeof(temp_config)))
        {
            Serial.println("No changes in configuration");
            return true;
        }

        Serial.println("Trying to save configuration");

        m_currentConfig.crc = calculateCrc();
        prefs.putBytes("configuration", &m_currentConfig, sizeof(m_currentConfig));

        Serial.println("Verifying configuration...");
        prefs.getBytes("configuration", &temp_config, sizeof(temp_config));
        if (0 == memcmp(&temp_config, &m_currentConfig, sizeof(temp_config)))
        {
            Serial.println("Configuration saved");
            Serial.printf("Configration CRC: 0x%08x\r\n", m_currentConfig.crc);
            m_configReady = true;
            return true;
        }

        retries--;
    } while (retries > 0);

    Serial.println("Failed to save configuration");
    return false;
}

uint32_t Configuration::calculateCrc()
{
    return esp_crc32_le(0u, (const uint8_t *)&m_currentConfig, sizeof(m_currentConfig) - 4u);
}

void Configuration::setMqttServerAddress(String &address)
{
    if (address.length() < sizeof(m_currentConfig.mqtt_server_address))
    {
        memset(m_currentConfig.mqtt_server_address,
               0,
               sizeof(m_currentConfig.mqtt_server_address));
        memcpy(m_currentConfig.mqtt_server_address,
               address.c_str(),
               sizeof(m_currentConfig.mqtt_server_address));
        m_configChanged = true;
    }
}

void Configuration::setMqttServerPort(uint16_t port)
{
    m_currentConfig.mqtt_server_port = port;
    m_configChanged = true;
}

void Configuration::setMqttServerLogin(String &login)
{
    if (login.length() < sizeof(m_currentConfig.mqtt_server_login))
    {
        memset(m_currentConfig.mqtt_server_login,
               0,
               sizeof(m_currentConfig.mqtt_server_login));
        memcpy(m_currentConfig.mqtt_server_login,
               login.c_str(),
               sizeof(m_currentConfig.mqtt_server_login));
        m_configChanged = true;
    }
}

void Configuration::setMqttServerPassword(String &password)
{
    if (password.length() < sizeof(m_currentConfig.mqtt_server_password))
    {
        memset(m_currentConfig.mqtt_server_password,
               0,
               sizeof(m_currentConfig.mqtt_server_password));
        memcpy(m_currentConfig.mqtt_server_password,
               password.c_str(),
               sizeof(m_currentConfig.mqtt_server_password));
        m_configChanged = true;
    }
}

void Configuration::setGpsReportInterval(const uint32_t interval)
{
    m_currentConfig.gps_report_interval = interval;
    m_configChanged = true;
}

void Configuration::setTemperatureReportInterval(const uint32_t interval)
{
    m_currentConfig.temperature_report_interval = interval;
    m_configChanged = true;
}

void Configuration::setBatteryReportInterval(const uint32_t interval)
{
    m_currentConfig.battery_report_interval = interval;
    m_configChanged = true;
}

void Configuration::setNetworkReportInterval(const uint32_t interval)
{
    m_currentConfig.network_report_interval = interval;
    m_configChanged = true;
}

void Configuration::setJaaleSensorAddress(String &address)
{
    if (address.length() < sizeof(m_currentConfig.jaale_sensor_address))
    {
        memset(m_currentConfig.jaale_sensor_address,
               0,
               sizeof(m_currentConfig.jaale_sensor_address));
        memcpy(m_currentConfig.jaale_sensor_address,
               address.c_str(),
               sizeof(m_currentConfig.jaale_sensor_address));
        m_configChanged = true;
    }
}

void Configuration::setDebugMode(const uint8_t mode)
{
    m_currentConfig.debug_mode = mode;
    m_configChanged = true;
}

void Configuration::cyclic()
{
    if (m_configChanged)
    {
        if (saveConfig())
        {
            m_configChanged = false;
        }
    }
}

void Configuration::defaultConfig()
{
    memset(&m_currentConfig.mqtt_server_address, 0, sizeof(m_currentConfig.mqtt_server_address));
    sprintf(m_currentConfig.mqtt_server_address, "iot.2canit.pl");
    memset(&m_currentConfig.mqtt_server_login, 0, sizeof(m_currentConfig.mqtt_server_login));
    memset(&m_currentConfig.mqtt_server_password, 0, sizeof(m_currentConfig.mqtt_server_password));
    m_currentConfig.mqtt_server_port = 1883u;
    m_currentConfig.battery_report_interval = 20000u;
    m_currentConfig.temperature_report_interval = 10000u;
    m_currentConfig.gps_report_interval = 10000u;
    m_currentConfig.network_report_interval = 10000u;
    m_currentConfig.debug_mode = 0u;
    sprintf(m_currentConfig.jaale_sensor_address, "de:96:36:e9:fc:09");

    Serial.println("Saving default configuration");

    saveConfig();
}

Configuration configuration;