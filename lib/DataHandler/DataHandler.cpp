#include "DataHandler.h"
#include <MqttController.h>
#include <Device.h>
#include <IrvineConfiguration.h>
#include <stdint.h>
#include <DataLogger.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

const char MODULE[] = "DATA_HDL";

void DataHandler::handleData(DataLoggable &data)
{
    char topic[50];
    sprintf(topic, "irvine/%s/measures/%s", irvineConfiguration.device.deviceId, data.logItem().c_str());

    mqttController.publish(topic, data.logMqttData().c_str());

    DataLoggerQueueItem logItem{
        .data = new DataLoggable(data)};
    xQueueSend(dataLoggerQueue, &logItem, 0);
}
