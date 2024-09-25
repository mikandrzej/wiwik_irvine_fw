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
    MqttTxItem mqttTxItem;

    logger.logPrintF(LogSeverity::DEBUG, MODULE, "irvine/%s/measures/%s", irvineConfiguration.device.deviceId, data.logItem().c_str());
    logger.logPrintF(LogSeverity::DEBUG, MODULE, "data: %s", data.logMqttData().c_str());

    snprintf(mqttTxItem.topic, sizeof(mqttTxItem.topic), "irvine/%s/measures/%s", irvineConfiguration.device.deviceId, data.logItem().c_str());
    strncpy(mqttTxItem.msg, data.logMqttData().c_str(), sizeof(mqttTxItem.msg));
    data.logMqttData().getBytes((unsigned char *)mqttTxItem.msg, sizeof(mqttTxItem.msg));

    modemManagement.mqttPublish(mqttTxItem);

    DataLoggerQueueItem logItem;
    data.logData().getBytes((unsigned char *)logItem.logData, sizeof(logItem.logData));
    data.logItem().getBytes((unsigned char *)logItem.logItem, sizeof(logItem.logItem));
    data.logMqttData().getBytes((unsigned char *)logItem.mqttData, sizeof(logItem.mqttData));

    // todo shared ptr?

    xQueueSendFromISR(dataLoggerQueue, &logItem, 0);
}
