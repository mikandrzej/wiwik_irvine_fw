#pragma once

#include <stdint.h>
#define TINY_GSM_MODEM_SIM7600 // A7670's AT instruction is compatible with SIM7600
#include <TinyGsmClient.h>

#define SerialAT Serial1
extern TinyGsmSim7600 modem;

enum class ModemManagementState
{
    CONFIGURING,
    OFF,
    POWERING_ON,
    BOOTING,
    SIM_UNLOCK,
    INIT_GPS,
    NETWORK_WAIT,
    CONNECT_APN,
    CONNECT_MQTT,
    ON,
    POWERING_OFF,
    SAFESTATE
};

enum class ModemExpectedPowerState
{
    OFF,
    SLEEP,
    ON
};

enum class ModemGpsState
{
    INIT,
    READY,
    FAIL
};

enum class ModemMqttState
{
    INIT,
    READY,
    NOT_READY,
    FAIL
};

using GpsStateCallback = void(const ModemGpsState &);
using MqttStateCallback = void(const ModemMqttState &);

class ModemManagement
{
public:
    ModemManagement(const uint32_t baudrate,
                    const uint8_t tx_pin,
                    const uint8_t rx_pin,
                    const uint8_t pwr_pin,
                    const uint8_t reset_pin,
                    const String pin_code,
                    const String apn,
                    const String apn_username,
                    const String apn_password,
                    const String mqtt_server,
                    const uint16_t mqtt_port,
                    const String mqtt_login,
                    const String mqtt_password,
                    const String mqtt_device_id);

    void loop(const uint32_t uptime_ms);
    void requestPowerState(const ModemExpectedPowerState req_state);
    void setModemGpsStateCallback(const GpsStateCallback *const callback);
    void setMqttStateCallback(const MqttStateCallback *const callback);

private:
    void setGpsState(ModemGpsState state);
    void setMqttState(ModemMqttState state);

private:
    ModemManagementState state = ModemManagementState::CONFIGURING;
    ModemExpectedPowerState expPowerState = ModemExpectedPowerState::OFF;

    uint32_t uart_baud;
    uint8_t txPin;
    uint8_t rxPin;
    uint8_t pwrPin;
    uint8_t resetPin;
    String pinCode;

    String modemName;
    String modemInfo;
    String simCcid;
    String simImsi;

    String apn = "internet";
    String apnUsername = "internet";
    String apnPassword = "internet";

    String mqttServer;
    uint16_t mqttServerPort;
    String mqttLogin;
    String mqttPassword;

    String mqttDeviceId;

    ModemGpsState gpsState = ModemGpsState::INIT;
    ModemMqttState mqttState = ModemMqttState::FAIL;

    const GpsStateCallback *gpsStateCallback = nullptr;
    const MqttStateCallback *mqttStateCallback = nullptr;
};