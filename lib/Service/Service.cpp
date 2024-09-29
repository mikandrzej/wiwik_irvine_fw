#include "Service.h"

#include "IrvineConfiguration.h"
#include <Logger.h>
#include <ArduinoJson.h>
#include <Updater.h>
#include <Vehicle.h>

const char MODULE[] = "SERVICE";

Service service;

void Service::begin()
{
    sprintf(mqttTopicSetConfig, "irvine/%s/service/set_config", irvineConfiguration.device.deviceId);
    mqttSubTopicSetConfig = new MqttSubscribedTopic(mqttTopicSetConfig, [this](char *topic, uint8_t *msg, unsigned int len)
                                                    { this->mqttSetConfigMessageReceived(topic, msg, len); });
    modemManagement.subscribe(mqttSubTopicSetConfig);

    sprintf(mqttTopicUpdate, "irvine/%s/service/update", irvineConfiguration.device.deviceId);
    mqttSubTopicUpdate = new MqttSubscribedTopic(mqttTopicUpdate, [this](char *topic, uint8_t *msg, unsigned int len)
                                                 { this->mqttUpdateMessageReceived(topic, msg, len); });
    modemManagement.subscribe(mqttSubTopicUpdate);

    sprintf(mqttTopicCalibration, "irvine/%s/service/calibration", irvineConfiguration.device.deviceId);
    mqttSubTopicCalibration = new MqttSubscribedTopic(mqttTopicCalibration, [this](char *topic, uint8_t *msg, unsigned int len)
                                                      { this->mqttCalibrationMessageReceived(topic, msg, len); });
    modemManagement.subscribe(mqttSubTopicCalibration);

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

void Service::mqttUpdateMessageReceived(char *topic, uint8_t *message, unsigned int length)
{
    // Convert the incoming message to a null-terminated string
    char jsonMessage[length + 1];
    strncpy(jsonMessage, (char *)message, length);
    jsonMessage[length] = '\0';

    logger.logPrintF(LogSeverity::INFO, MODULE, "Mqtt Update message: %s / %s", topic, jsonMessage);

    // Parse the JSON message
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, jsonMessage);

    if (error)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "Failed to parse JSON: %s", error.c_str());
        return;
    }
    if (doc.containsKey("url"))
    {
        updater.updateTrigger(doc["url"].as<const char *>());
    }
}

void Service::mqttCalibrationMessageReceived(char *topic, uint8_t *message, unsigned int length)
{
    // Convert the incoming message to a null-terminated string
    char jsonMessage[length + 1];
    strncpy(jsonMessage, (char *)message, length);
    jsonMessage[length] = '\0';

    logger.logPrintF(LogSeverity::INFO, MODULE, "Mqtt Calibration message: %s / %s", topic, jsonMessage);

    // Parse the JSON message
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, jsonMessage);

    if (error)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "Failed to parse JSON: %s", error.c_str());
        return;
    }
    if (doc.containsKey("set_voltage"))
    {
        float ref_value = doc["set_voltage"].as<float>();
        batteryCalibration(ref_value);
    }
}

bool Service::batteryCalibration(const float reference_voltage)
{
    bool valid = false;
    float meas_value = vehicle.getVccVoltage(&valid, true);
    if (valid)
    {
        float calibration_scale = reference_voltage / meas_value;
        if (irvineConfiguration.setParameter("dev.batCalib", calibration_scale))
        {
            logger.logPrintF(LogSeverity::INFO, MODULE, "Calibration done. Scale: %f", calibration_scale);
            return true;
        }
    }
    return false;
}
