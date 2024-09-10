#include "DataHandler.h"
#include <ModemManagement.h>
#include <Device.h>
#include <IrvineConfiguration.h>
#include <stdint.h>
#include <DataLogger.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

const char MODULE[] = "DATA_HDL";

void DataHandler::handleData(DataLoggable &data)
{
    char topic[100];

    logger.logPrintF(LogSeverity::DEBUG, MODULE, "irvine/%s/measures/%s", irvineConfiguration.device.deviceId, data.logItem().c_str());
    logger.logPrintF(LogSeverity::DEBUG, MODULE, "data: %s", data.logMqttData().c_str());

    sprintf(topic, "irvine/%s/measures/%s", irvineConfiguration.device.deviceId, data.logItem().c_str());

    modemManagement.mqttPublish(topic, data.logMqttData().c_str());

    DataLoggerQueueItem logItem;
    data.logData().getBytes((unsigned char *)logItem.logData, sizeof(logItem.logData));
    data.logItem().getBytes((unsigned char *)logItem.logItem, sizeof(logItem.logItem));
    data.logMqttData().getBytes((unsigned char *)logItem.mqttData, sizeof(logItem.mqttData));

    // todo shared ptr?

    xQueueSend(dataLoggerQueue, &logItem, 0);
}
