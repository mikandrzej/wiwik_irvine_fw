#pragma once

#include <stdint.h>
#include <ModemManagement.h>
#include <MqttTxItem.h>

#define MAX_CHUNKS 50u
#define MAX_FILENAME_LEN 20u

enum class UpdateState
{
    IDLE,
    TRIGGERED,
    OBTAIN_CHUNK_LIST,
    GET_CHUNK,
    WRITE_CHUNK,
    FINISHING,
    BEFORE_RESET
};

class Updater
{
public:
    void loop();
    bool updateTrigger(const char *url);

private:
    void updateProgress(const float progress);
    void updateStatus(const char *format, ...);

    void updateError(const char *format, ...);

    bool buildChunkUrl(const char *origin, const char *chunkFilename, char *url);

    UpdateState updateState = UpdateState::IDLE;

    uint8_t buf[4096];
    char url[128];

    char chunkFiles[MAX_CHUNKS][MAX_FILENAME_LEN];
    uint16_t currentChunk;
    uint16_t chunkCnt;

    int currentUpdatePosition = 0u;
    int currentChunkPosition;

    int chunkLength;
    int totalLength;

    uint32_t resetTimestamp;

    MqttTxItem mqttTxItem;
};

extern Updater updater;