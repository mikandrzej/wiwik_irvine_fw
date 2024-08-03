#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <Arduino.h>

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

    SemaphoreHandle_t semaphore;

private:
    ModemManagementState state = ModemManagementState::UNINITIALIZED;
    String modemName;
    String modemInfo;
    String simCcid;
    String simImsi;
};

extern ModemManagement modemManagement;