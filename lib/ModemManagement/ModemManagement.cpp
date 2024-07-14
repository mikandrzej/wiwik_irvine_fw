#include "ModemManagement.h"

#include <Arduino.h>

#define TINY_GSM_MODEM_SIM7600 // A7670's AT instruction is compatible with SIM7600
#include <TinyGsmClient.h>

TinyGsmSim7600 modem(SerialAT);

ModemManagement::ModemManagement(const uint32_t baudrate,
                                 const uint8_t tx_pin,
                                 const uint8_t rx_pin,
                                 const uint8_t pwr_pin,
                                 const uint8_t reset_pin,
                                 const String pin_code)
{
    this->uart_baud = baudrate;
    this->tx_pin = tx_pin;
    this->rx_pin = rx_pin;
    this->pwr_pin = pwr_pin;
    this->reset_pin = reset_pin;
    this->pin_code = pin_code;
}

void ModemManagement::loop(const uint32_t uptime_ms)
{
    switch (state)
    {
    case ModemManagementState::CONFIGURING:
        SerialAT.begin(uart_baud, SERIAL_8N1, rx_pin, tx_pin);

        pinMode(pwr_pin, OUTPUT);
        digitalWrite(pwr_pin, LOW);

        pinMode(reset_pin, OUTPUT);
        digitalWrite(reset_pin, LOW);
        state = ModemManagementState::OFF;
        break;

    case ModemManagementState::OFF:
        if (ModemExpectedState::SLEEP == exp_state || ModemExpectedState::ON == exp_state)
        {
            state = ModemManagementState::POWERING_ON;
        }
        break;

    case ModemManagementState::POWERING_ON:
        digitalWrite(reset_pin, HIGH);
        vTaskDelay(pdMS_TO_TICKS(3000));
        digitalWrite(pwr_pin, HIGH);
        vTaskDelay(pdMS_TO_TICKS(1000));
        digitalWrite(pwr_pin, LOW);
        vTaskDelay(pdMS_TO_TICKS(3000));
        state = ModemManagementState::BOOTING;
        break;

    case ModemManagementState::BOOTING:
        if (modem.init())
        {
            modem_name = modem.getModemName();
            modem_info = modem.getModemInfo();

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
            if (modem.simUnlock(pin_code.c_str()))
            {
                state = ModemManagementState::NETWORK_WAIT;
            }
            else
            {
                state = ModemManagementState::SAFESTATE;
            }
        }
        else if (sim_status == SIM_READY)
        {
            state = ModemManagementState::NETWORK_WAIT;
        }
        else
        {
            state = ModemManagementState::SAFESTATE;
        }
        break;

    case ModemManagementState::NETWORK_WAIT:
        if (modem.isNetworkConnected())
        {
            sim_ccid = modem.getSimCCID();
            sim_imsi = modem.getIMSI();
        }
        else if (ModemExpectedState::OFF == exp_state)
        {
            state = ModemManagementState::POWERING_OFF;
        }
        break;

    case ModemManagementState::ON:
    
        break;

    case ModemManagementState::POWERING_OFF:
        digitalWrite(pwr_pin, LOW);
        digitalWrite(reset_pin, LOW);
        vTaskDelay(pdMS_TO_TICKS(3000));
        state = ModemManagementState::OFF;
        break;
    }
}

void ModemManagement::requestState(const ModemExpectedState req_state)
{
    exp_state = req_state;
}
