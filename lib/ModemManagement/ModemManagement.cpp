#include "ModemManagement.h"

#include <Arduino.h>

#define TINY_GSM_MODEM_SIM7600 // A7670's AT instruction is compatible with SIM7600
#include <TinyGsmClient.h>
#include <PubSubClient.h>

TinyGsmSim7600 modem(SerialAT);
PubSubClient mqtt(client);
TinyGsmClient client(modem, 0);

ModemManagement::ModemManagement(const uint32_t baudrate,
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
                                 const String mqtt_device_id)
{
    this->uart_baud = baudrate;
    this->txPin = tx_pin;
    this->rxPin = rx_pin;
    this->pwrPin = pwr_pin;
    this->resetPin = reset_pin;
    this->pinCode = pin_code;
    this->apn = apn;
    this->apnUsername = apn_username;
    this->apnPassword = apn_password;
    this->mqttServer = mqtt_server;
    this->mqttServerPort = mqtt_port;
    this->mqttLogin = mqtt_login;
    this->mqttPassword = mqtt_password;
    this->mqttDeviceId = mqtt_device_id;
}

void ModemManagement::loop(const uint32_t uptime_ms)
{
    switch (state)
    {
    case ModemManagementState::CONFIGURING:
        SerialAT.begin(uart_baud, SERIAL_8N1, rxPin, txPin);

        pinMode(pwrPin, OUTPUT);
        digitalWrite(pwrPin, LOW);

        pinMode(resetPin, OUTPUT);
        digitalWrite(resetPin, LOW);

        mqtt.setServer(mqttServer.c_str(), mqttServerPort);

        state = ModemManagementState::OFF;
        break;

    case ModemManagementState::OFF:
        if (ModemExpectedPowerState::SLEEP == expPowerState || ModemExpectedPowerState::ON == expPowerState)
        {
            state = ModemManagementState::POWERING_ON;
        }
        break;

    case ModemManagementState::POWERING_ON:
        digitalWrite(resetPin, HIGH);
        vTaskDelay(pdMS_TO_TICKS(3000));
        digitalWrite(pwrPin, HIGH);
        vTaskDelay(pdMS_TO_TICKS(1000));
        digitalWrite(pwrPin, LOW);
        vTaskDelay(pdMS_TO_TICKS(3000));
        state = ModemManagementState::BOOTING;
        break;

    case ModemManagementState::BOOTING:
        if (modem.init())
        {
            modemName = modem.getModemName();
            modemInfo = modem.getModemInfo();

            state = ModemManagementState::SIM_UNLOCK;
        }
        else
        {
            state = ModemManagementState::POWERING_OFF;
        }
        break;

    case ModemManagementState::SIM_UNLOCK:
        SimStatus sim_status = modem.getSimStatus(2000);
        if (sim_status == SIM_LOCKED)
        {
            if (modem.simUnlock(pinCode.c_str()))
            {
                simCcid = modem.getSimCCID();
                simImsi = modem.getIMSI();
                state = ModemManagementState::INIT_GPS;
            }
            else
            {
                state = ModemManagementState::SAFESTATE;
            }
        }
        else if (sim_status == SIM_READY)
        {
            simCcid = modem.getSimCCID();
            simImsi = modem.getIMSI();
            state = ModemManagementState::INIT_GPS;
        }
        else
        {
            state = ModemManagementState::SAFESTATE;
        }
        break;

    case ModemManagementState::INIT_GPS:
        if (ModemGpsState::INIT == gpsState)
        {
            ModemGpsState newGpsState = ModemGpsState::FAIL;

            modem.sendAT("+CGNSSPWR=0");
            if (modem.waitResponse(1000) == 1)
            {
                modem.sendAT("+CGNSSPWR=1");
                if (modem.waitResponse(1000) == 1)
                {
                    if (modem.waitResponse(20000UL, "+CGNSSPWR: READY!") == 1)
                    {
                        modem.sendAT("+CGNSSMODE=3");
                        if (modem.waitResponse(1000) == 1)
                        {
                            newGpsState = ModemGpsState::READY;
                        }
                    }
                }
            }

            setGpsState(newGpsState);
        }
        state = ModemManagementState::NETWORK_WAIT;
        break;

    case ModemManagementState::NETWORK_WAIT:
        if (modem.isNetworkConnected())
        {
            state = ModemManagementState::CONNECT_APN;
        }
        break;

    case ModemManagementState::CONNECT_APN:
        if (!modem.isNetworkConnected())
        {
            state = ModemManagementState::NETWORK_WAIT;
        }
        else if (modem.gprsConnect(apn.c_str(), apnUsername.c_str(), apnPassword.c_str()))
        {
            state = ModemManagementState::CONNECT_MQTT;
        }
        break;

    case ModemManagementState::CONNECT_MQTT:
        if (!modem.isGprsConnected())
        {
            state = ModemManagementState::CONNECT_APN;
        }
        else
        {
            bool result;
            if (mqttLogin.length() != 0)
            {
                result = mqtt.connect(mqttDeviceId.c_str());
            }
            else
            {
                result = mqtt.connect(mqttDeviceId.c_str(), mqttLogin.c_str(), mqttPassword.c_str());
            }

            if (result)
            {
                setMqttState(ModemMqttState::READY);
                state = ModemManagementState::ON;
            }
            else
            {
                setMqttState(ModemMqttState::FAIL);
            }
        }
        break;

    case ModemManagementState::ON:
        if (expPowerState)
        if (!modem.isNetworkConnected())
        {
            setMqttState(ModemMqttState::NOT_READY);
            state = ModemManagementState::NETWORK_WAIT;
        }
        else if (!modem.isGprsConnected())
        {
            setMqttState(ModemMqttState::NOT_READY);
            state = ModemManagementState::CONNECT_APN;
        }
        else if (!mqtt.connected())
        {
            setMqttState(ModemMqttState::NOT_READY);
            state = ModemManagementState::CONNECT_MQTT;
        }
        break;

    case ModemManagementState::POWERING_OFF:
        digitalWrite(pwrPin, LOW);
        digitalWrite(resetPin, LOW);
        vTaskDelay(pdMS_TO_TICKS(3000));
        state = ModemManagementState::OFF;
        break;

    case ModemManagementState::SAFESTATE:
        break;
    }
}

void ModemManagement::requestPowerState(const ModemExpectedPowerState req_state)
{
    expPowerState = req_state;
}

void ModemManagement::setModemGpsStateCallback(const GpsStateCallback *const callback)
{
    gpsStateCallback = callback;
}

void ModemManagement::setMqttStateCallback(const MqttStateCallback *const callback)
{
    mqttStateCallback = callback;
}

void ModemManagement::setGpsState(ModemGpsState state)
{
    gpsState = state;
    if (gpsStateCallback)
    {
        gpsStateCallback(gpsState);
    }
}

void ModemManagement::setMqttState(ModemMqttState state)
{
    mqttState = state;
    if (mqttStateCallback)
    {
        mqttStateCallback(mqttState);
    }
}
