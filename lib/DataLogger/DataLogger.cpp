#include "DataLogger.h"
#include <TimeLib.h>

#include <FS.h>
#include <SD.h>

#include "Logger.h"
#include <HwConfiguration.h>
#include <MqttPublisher.h>

const char MODULE[] = "D_LOGGER";

#define DATA_LOGGER_QUEUE_LENGTH 5
#define DATA_LOGGER_ITEM_SIZE sizeof(DataLoggerQueueItem)
// static uint8_t dataLoggerQueueBuffer[DATA_LOGGER_QUEUE_LENGTH * DATA_LOGGER_ITEM_SIZE];
// static StaticQueue_t dataLoggerStaticQueue;
QueueHandle_t dataLoggerQueue;

DataLogger dataLogger;

void DataLogger::begin()
{
    /* Create a queue capable of containing 10 uint64_t values. */
    // dataLoggerQueue = xQueueCreateStatic(DATA_LOGGER_QUEUE_LENGTH,
    //                                      DATA_LOGGER_ITEM_SIZE,
    //                                      dataLoggerQueueBuffer,
    //                                      &dataLoggerStaticQueue);
    dataLoggerQueue = xQueueCreate(DATA_LOGGER_QUEUE_LENGTH, DATA_LOGGER_ITEM_SIZE);

    SPI.begin(BOARD_SCK_PIN, BOARD_MISO_PIN, BOARD_MOSI_PIN);
    pinMode(BOARD_SD_CS_PIN, OUTPUT);

    if (!SD.begin(BOARD_SD_CS_PIN, SPI))
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

    cardSize = SD.cardSize();
    logger.logPrintF(LogSeverity::INFO, MODULE, "SD Card Size: %lluMB\n", cardSize / (1024 * 1024));

    uint64_t used_space = SD.usedBytes();
    uint64_t total_space = SD.totalBytes();
    logger.logPrintF(LogSeverity::INFO, MODULE, "SD Card space: %llu / %lluMB\n", used_space / (1024 * 1024), total_space / (1024 * 1024));

    if (!SD.mkdir(logPathPrefix))
    {
        logger.logPrintF(LogSeverity::INFO, MODULE, "Failed to create directory %s", logPathPrefix.c_str());
        return;
    }

    logger.logPrintF(LogSeverity::INFO, MODULE, "Module init OK");

    initialized = true;

    MqttPublisher::publishSdData(used_space, total_space, cardSize);
}

void DataLogger::logData(DataLoggerQueueItem &data)
{
    if (!initialized)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "log data error - SD not initialized");
        return;
    }

    String path = logPathPrefix + "/" + String(data.logItem) + ".csv";

    fs::File file = SD.open(path.c_str(), FILE_APPEND);
    if (!file)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "Failed to open file %s", path.c_str());
        return;
    }

    uint32_t logDataLen = strlen(data.logData);
    size_t savedLen = file.write((uint8_t *)data.logData, logDataLen);
    savedLen += file.write((const uint8_t *)"\r\n", 2);

    file.flush();

    if (savedLen == logDataLen + 2u)
    {
        logger.logPrintF(LogSeverity::DEBUG, MODULE, "Logged data");
    }
    else
    {
        int error = file.getWriteError();
        logger.logPrintF(LogSeverity::ERROR, MODULE, "Data log error %d, logged: %d, expexted: %d", error, savedLen, logDataLen + 2u);
    }

    file.close();
}

void DataLogger::reopenFile(PathFileData *fileData)
{
    fileData->file.close();
    fileData->file = SD.open(fileData->path, FILE_APPEND);
    logger.logPrintF(LogSeverity::INFO, MODULE, "Reopened file %s", fileData->path.c_str());
}