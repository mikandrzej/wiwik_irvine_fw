#pragma once

#include <Arduino.h>
#include <FS.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <stdint.h>

#include <DataLoggable.h>

typedef struct
{
    char logItem[50];
    char logData[200];
    char mqttData[200];
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
    void logData(DataLoggerQueueItem &data);

    PathFileData *getFileData(String &path);


private:
    void reopenFile(PathFileData *fileData);
    String logPathPrefix = "/log";

    int linesToSave = {0};

    uint8_t cardType;
    uint64_t cardSize;
    bool initialized;

    std::vector<PathFileData *> pathFiles{};
};

extern DataLogger dataLogger;
extern QueueHandle_t dataLoggerQueue;