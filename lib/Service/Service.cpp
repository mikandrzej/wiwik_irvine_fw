#include "Service.h"

#include "IrvineConfiguration.h"
#include <Logger.h>
#include <ArduinoJson.h>

const char MODULE[] = "SERVICE";

Service service;

void Service::begin()
{
    sprintf(mqttTopicSetConfig, "irvine/%s/service/set_config", irvineConfiguration.device.deviceId);
    mqttSubTopicSetConfig = new MqttSubscribedTopic(mqttTopicSetConfig, [this](char *topic, uint8_t *msg, unsigned int len)
                                                    { this->mqttSetConfigMessageReceived(topic, msg, len); });
    modemManagement.subscribe(mqttSubTopicSetConfig);

    logger.logPrintF(LogSeverity::INFO, MODULE, "Module started");
}

void Service::mqttSetConfigMessageReceived(char *topic, uint8_t *message, unsigned int length)
{
    // Convert the incoming message to a null-terminated string
    char jsonMessage[length + 1];
    strncpy(jsonMessage, (char *)message, length);
    jsonMessage[length] = '\0';

    logger.logPrintF(LogSeverity::INFO, MODULE, "Set Config mqtt message: %s / %s", topic, jsonMessage);

    // Parse the JSON message
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, jsonMessage);

    if (error)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "Failed to parse JSON: %s", error.c_str());
        return;
    }

    // Iterate over the JSON object and set each configuration parameter
    for (JsonPair kv : doc.as<JsonObject>())
    {
        const char *paramName = kv.key().c_str();
        const char *paramValue = kv.value().as<const char *>();

        if (!irvineConfiguration.setParameter(paramName, paramValue))
        {
            logger.logPrintF(LogSeverity::ERROR, MODULE, "Failed to set parameter: %s to %s", paramName, paramValue);
        }
        else
        {
            logger.logPrintF(LogSeverity::INFO, MODULE, "Parameter set: %s = %s", paramName, paramValue);
        }
    }
}