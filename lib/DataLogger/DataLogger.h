#pragma once

#include <Arduino.h>
#include <FS.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <stdint.h>

#include <DataLoggable.h>

typedef struct
{
    DataLoggable *data;
} DataLoggerQueueItem;

typedef struct
{
    String path;
    fs::File file;
} PathFileData;

class DataLogger
{
public:
    void begin();

    void logData(DataLoggable *data);

    fs::File *getFile(String &path);

private:
    String logPathPrefix = "/log/";

    int linesToSave = {0};

    uint8_t cardType;
    uint64_t cardSize;
    bool initialized;

    std::vector<PathFileData *> pathFiles{};
};

extern DataLogger dataLogger;
extern QueueHandle_t dataLoggerQueue;