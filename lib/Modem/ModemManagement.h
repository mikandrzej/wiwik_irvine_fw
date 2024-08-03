#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <Arduino.h>
#include <EgTinyGsm.h>

extern SemaphoreHandle_t modemSemaphore;

extern EgTinyGsm modem;

enum class ModemManagementState
{
    UNINITIALIZED,
    POWER_OFF,
    APN_DISCONNECTED,
    INIT_APN,
    INIT_GPS,
    APN_CONNECTED,
    INFINITE_LOOP
};

class ModemManagement
{
public:
    ModemManagement();
    void begin();
    void loop();

    bool isConnected();

private:
    ModemManagementState state = ModemManagementState::UNINITIALIZED;
    String modemName;
    String modemInfo;
    String simCcid;
    String simImsi;
    bool connected;
};

extern ModemManagement modemManagement;