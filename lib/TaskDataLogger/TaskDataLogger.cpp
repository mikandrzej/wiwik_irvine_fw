#include "TaskDataLogger.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <DataLogger.h>
#include <Logger.h>
#include <memory>

const char MODULE[] = "DATA_LOG";

void taskDataLogger(void *)
{
    DataLoggerQueueItem receivedItem;
    while (1)
    {
        if (xQueueReceive(dataLoggerQueue,
                          &receivedItem,
                          portMAX_DELAY) == pdPASS)
        {
            dataLogger.logData(receivedItem);
        }
    }
}