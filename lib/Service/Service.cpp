#include "Service.h"

#include "IrvineConfiguration.h"
#include <Logger.h>

const char MODULE[] = "SERVICE";

Service service;

void Service::begin()
{
    char *topic = (char *)malloc(100);
    if (topic)
    {
        sprintf(topic, "%s/service/#", irvineConfiguration.device.deviceId);
        mqttSubscribedTopic = new MqttSubscribedTopic(topic, [this](char *topic, uint8_t *msg, unsigned int len)
                                                      { this->mqttMessageReceived(topic, msg, len); });
        logger.logPrintF(LogSeverity::INFO, MODULE, "Module started");
    }
}

void Service::mqttMessageReceived(char *topic, uint8_t *message, unsigned int length)
{
    logger.logPrintF(LogSeverity::INFO, MODULE, "Received mqtt message: %s / %s", topic, (char *)message);
}
