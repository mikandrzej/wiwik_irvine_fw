#pragma once

#include <Arduino.h>
#include <FS.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <GpsData.h>
#include <JaaleeData.h>

#include <stdint.h>

enum class DataLoggerLogType
{
    GPS,
    JAALEE,
    MDOEM
};

typedef struct
{
    DataLoggerLogType type;
    uint64_t unixTimestamp;
    union
    {
        JaaleeData jaalee;
        GpsData gps;
    };
} DataLoggerQueueItem;

class DataLogger
{
public:
    void begin();

    void logData(const uint64_t unixTimestamp, const GpsData &gpsData);
    void logData(const uint64_t unixTimestamp, const JaaleeData &jaaleeData);

private:
    void saveData();

private:
    String logFilePath = "/log/log.csv";

    fs::File file;

    int linesToSave = {0};

    uint8_t cardType;
    uint64_t cardSize;
    bool initialized;
};

extern DataLogger dataLogger;
extern QueueHandle_t dataLoggerQueue;