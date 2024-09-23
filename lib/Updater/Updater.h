#pragma once

#include <stdint.h>
#include <ModemManagement.h>
#include <MqttTxItem.h>

enum class UpdateState
{
    IDLE,
    TRIGGERED,
    IN_PROGRESS,
    FINISHING,
    BEFORE_RESET
};

class Updater
{
public:
    void loop();
    bool updateTrigger(const char *url);

private:
    void sendUpdateProgress(const float progress);
    void sendUpdateStatus(const char *status);

    uint8_t buf[4096];
    UpdateState updateState = UpdateState::IDLE;
    uint32_t currentAddress = 0u;
    uint32_t newFileLen = 0u;
    uint32_t resetTimestamp;
    MqttTxItem mqttTxItem;
};

extern Updater updater;