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
    NETWORK_WAIT,
    ON,
    POWERING_OFF,
    SAFESTATE
};

enum class ModemExpectedState
{
    OFF,
    SLEEP,
    ON
};

class ModemManagement
{
public:
    ModemManagement(const uint32_t baudrate,
                    const uint8_t tx_pin,
                    const uint8_t rx_pin,
                    const uint8_t pwr_pin,
                    const uint8_t reset_pin,
                    const String pin_code);

    void loop(const uint32_t uptime_ms);
    void requestState(const ModemExpectedState req_state);

private:
    void modemInitState();

private:
    ModemManagementState state = ModemManagementState::CONFIGURING;
    ModemExpectedState exp_state = ModemExpectedState::OFF;

    uint32_t uart_baud;
    uint8_t tx_pin;
    uint8_t rx_pin;
    uint8_t pwr_pin;
    uint8_t reset_pin;
    String pin_code;

    String modem_name;
    String modem_info;
    String sim_ccid;
    String sim_imsi;
};