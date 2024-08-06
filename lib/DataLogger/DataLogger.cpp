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

    logger.logPrintF(LogSeverity::INFO, MODULE, "Module init OK");
    initialized = true;
}

void DataLogger::logData(DataLoggable *data)
{
    if (!initialized)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "log data error - SD not initialized");
        return;
    }

    String path = logPathPrefix + data->logItem() + ".csv";
    auto *fileData = getFileData(path);

    if (fileData == nullptr)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "log data error - File error");
        return;
    }
    uint32_t logDataLen = data->logData().length();

    size_t savedLen = fileData->file.write((uint8_t *)data->logData().c_str(), logDataLen);

    fileData->file.flush();

    if (savedLen == logDataLen)
    {
        logger.logPrintF(LogSeverity::DEBUG, MODULE, "Logged data");
    }
    else
    {
        int error = fileData->file.getWriteError();
        logger.logPrintF(LogSeverity::DEBUG, MODULE, "Data log error %d, logged: %d, expexted: %d", error, savedLen, logDataLen);

        reopenFile(fileData);
        savedLen = fileData->file.write((uint8_t *)data->logData().c_str(), logDataLen);
        if (savedLen == logDataLen)
        {
            logger.logPrintF(LogSeverity::DEBUG, MODULE, "Save retry succeed");
        }
    }
}

PathFileData *DataLogger::getFileData(String &path)
{
    if (!initialized)
    {
        return nullptr;
    }
    for (auto *filePathData : pathFiles)
    {
        if (filePathData->path.compareTo(path) == 0)
        {
            return filePathData;
        }
    }

    auto *fpData = new PathFileData{
        .path = path,
        .file = SD.open(path.c_str(), FILE_APPEND),
    };
    pathFiles.push_back(fpData);

    logger.logPrintF(LogSeverity::INFO, MODULE, "Opened new file %s", path.c_str());

    return fpData;
}

void DataLogger::reopenFile(PathFileData *fileData)
{
    fileData->file.close();
    fileData->file = SD.open(fileData->path, FILE_APPEND);
    logger.logPrintF(LogSeverity::INFO, MODULE, "Reopened file %s", fileData->path.c_str());
}
