#include "TaskDataLogger.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <DataLogger.h>
#include <Logger.h>

const char MODULE[] = "DATA_LOG";

void taskDataLogger(void *)
{
    DataLoggerQueueItem receivedItem;
    while (1)
    {
        if (xQueueReceive(dataLoggerQueue,
                          &receivedItem,
                          pdMS_TO_TICKS(10000)) == pdPASS)
        {
            switch (receivedItem.type)
            {
            case DataLoggerLogType::GPS:
                dataLogger.logData(receivedItem.unixTimestamp, receivedItem.gps);
                break;
            case DataLoggerLogType::JAALEE:
                dataLogger.logData(receivedItem.unixTimestamp, receivedItem.jaalee);
                break;
            default:
                logger.logPrintF(LogSeverity::INFO, MODULE, "Unknown log type %d", static_cast<int>(receivedItem.type));
                break;
            }
        }
    }
}