#include "DataLogger.h"
#include <TimeLib.h>

#include <FS.h>
#include <SD.h>

#include "Logger.h"
#include <HwConfiguration.h>

const char MODULE[] = "D_LOGGER";

#define DATA_LOGGER_QUEUE_LENGTH 10
#define DATA_LOGGER_ITEM_SIZE sizeof(DataLoggerQueueItem)
static uint8_t dataLoggerQueueBuffer[DATA_LOGGER_QUEUE_LENGTH * DATA_LOGGER_ITEM_SIZE];
static StaticQueue_t dataLoggerStaticQueue;
QueueHandle_t dataLoggerQueue;

DataLogger dataLogger;

void DataLogger::begin()
{
    /* Create a queue capable of containing 10 uint64_t values. */
    dataLoggerQueue = xQueueCreateStatic(DATA_LOGGER_QUEUE_LENGTH,
                                         DATA_LOGGER_ITEM_SIZE,
                                         dataLoggerQueueBuffer,
                                         &dataLoggerStaticQueue);

    if (!SD.begin(SD_CS_PIN, SPI))
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "SD init failed");
        return;
    }

    cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        logger.logPrintF(LogSeverity::WARNING, MODULE, "No SD card attached");
        return;
    }

    const char *cardTypeStr = nullptr;
    switch (cardType)
    {
    case CARD_MMC:
        cardTypeStr = "MMC";
        break;
    case CARD_SD:
        cardTypeStr = "SDSC";
        break;
    case CARD_SDHC:
        cardTypeStr = "SDHC";
        break;
    default:
        cardTypeStr = "UNKNOWN";
        break;
    }
    logger.logPrintF(LogSeverity::INFO, MODULE, "SD card type: %s", cardTypeStr);

    cardSize = SD.cardSize() / (1024 * 1024);
    logger.logPrintF(LogSeverity::INFO, MODULE, "SD Card Size: %lluMB\n", cardSize);

    file = SD.open(logFilePath, FILE_APPEND, true);
    logger.logPrintF(LogSeverity::INFO, MODULE, "Module init OK");
    initialized = true;
}

void DataLogger::logData(const uint64_t unixTimestamp, const GpsData &gpsData)
{
    if (!initialized)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "log GPS data error - SD not initialized");
        return;
    }
    char line[300];
    size_t len = sprintf(line, "%ull;%u;%u;%f;%f;%f;%f;%ull\r\n",
                         unixTimestamp,
                         gpsData.mode,
                         gpsData.satellites,
                         gpsData.latitude,
                         gpsData.longitude,
                         gpsData.altitude,
                         gpsData.speed,
                         gpsData.unixTimestamp);
    size_t savedLen = file.write((uint8_t *)line, len);
    if (savedLen == len)
    {
        logger.logPrintF(LogSeverity::DEBUG, MODULE, "Logged GPS data");
    }
    else
    {
        logger.logPrintF(LogSeverity::DEBUG, MODULE, "GPS data log error");
    }
    saveData();
}

void DataLogger::logData(const uint64_t unixTimestamp, const JaaleeData &jaaleeData)
{
    if (!initialized)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "log GPS data error - SD not initialized");
        return;
    }
    char line[200];
    size_t len = sprintf(line, "%ull;%f;%f;%f;%d\r\n",
                         unixTimestamp,
                         jaaleeData.temperature,
                         jaaleeData.humidity,
                         jaaleeData.battery,
                         jaaleeData.rssi);
    size_t savedLen = file.write((uint8_t *)line, len);
    if (savedLen == len)
    {
        logger.logPrintF(LogSeverity::DEBUG, MODULE, "Logged Jaalee data");
    }
    else
    {
        logger.logPrintF(LogSeverity::DEBUG, MODULE, "Jaalee data log error");
    }
    saveData();
}

void DataLogger::saveData()
{
    linesToSave++;

    if (linesToSave > 0)
    {
        file.flush();
        linesToSave = 0;

        logger.logPrintF(LogSeverity::DEBUG, MODULE, "File flushed");
    }
}
