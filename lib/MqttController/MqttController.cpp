#include "MqttController.h"

#include <IrvineConfiguration.h>
#include <ModemManagement.h>
#include <Logger.h>

MqttController mqttController;

const char MODULE[] = "MQTT";

MqttSubscribedTopic::MqttSubscribedTopic(char *topic, MqttCallback callback) : topic(topic), callback(callback)
{
    topicLength = strlen(topic);
}

void MqttController::begin()
{
    client = new TinyGsmClient(modem, 0);
    mqtt = new PubSubClient(*client);
    mqtt->setServer(irvineConfiguration.server.mqttHost, irvineConfiguration.server.mqttPort);
    mqtt->setCallback([this](char *topic, uint8_t *msg, unsigned int len)
                      { this->messageCallback(topic, msg, len); });
}

void MqttController::loop()
{
    // todo semaphore
    switch (state)
    {
    case MqttState::DISCONNECTED:
        if (irvineConfiguration.server.mqttHost[0u] != '\0')
        {
            state = MqttState::CONNECTING;
        }
        break;

    case MqttState::CONNECTING:
        if (xSemaphoreTake(modemSemaphore, (TickType_t)10) == pdTRUE)
        {
            if (modemManagement.isConnected())
            {
                bool result;
                if (irvineConfiguration.server.mqttHost[0u] != '\0')
                {
                    logger.logPrintF(LogSeverity::INFO, MODULE, "Connecting to MQTT %s:%d with %s/%s as %s",
                                     irvineConfiguration.server.mqttHost,
                                     irvineConfiguration.server.mqttPort,
                                     irvineConfiguration.server.mqttUsername,
                                     irvineConfiguration.server.mqttPassword,
                                     irvineConfiguration.device.deviceId);
                    result = mqtt->connect(irvineConfiguration.device.deviceId,
                                           irvineConfiguration.server.mqttUsername,
                                           irvineConfiguration.server.mqttPassword);
                }
                else
                {
                    logger.logPrintF(LogSeverity::WARNING, MODULE, "Connecting to MQTT %s:%d without credetials as %s",
                                     irvineConfiguration.server.mqttHost,
                                     irvineConfiguration.server.mqttPort,
                                     irvineConfiguration.device.deviceId);
                    result = mqtt->connect(irvineConfiguration.device.deviceId);
                }

                if (result)
                {
                    logger.logPrintF(LogSeverity::INFO, MODULE, "Connected to MQTT");

                    for (auto &subscribedTopic : subscribedTopics)
                    {
                        result = mqtt->subscribe(subscribedTopic->topic);
                        if (result)
                        {
                            logger.logPrintF(LogSeverity::INFO, MODULE, "Succesfully subscribed topic %s", subscribedTopic->topic);
                        }
                        else
                        {
                            logger.logPrintF(LogSeverity::ERROR, MODULE, "Failed to subscribe topic %s", subscribedTopic->topic);
                            break;
                        }
                    }
                    if (result)
                    {
                        state = MqttState::CONNECTED;
                    }
                }
                else
                {
                    logger.logPrintF(LogSeverity::INFO, MODULE, "Connecting to MQTT failed");
                }
            }
            xSemaphoreGive(modemSemaphore);
        }
        break;

    case MqttState::CONNECTED:
        if (xSemaphoreTake(modemSemaphore, (TickType_t)10) == pdTRUE)
        {
            if (modemManagement.isConnected() && mqtt->connected())
            {
                mqtt->loop();
            }
            else
            {
                state = MqttState::CONNECTING;
            }
            xSemaphoreGive(modemSemaphore);
        }
        break;

    case MqttState::DISCONNECT:
        if (xSemaphoreTake(modemSemaphore, (TickType_t)10) == pdTRUE)
        {
            mqtt->disconnect();
            state = MqttState::DISCONNECTED;

            xSemaphoreGive(modemSemaphore);
        }
    }

    if (xSemaphoreTake(modemSemaphore, (TickType_t)10) == pdTRUE)
    {
        mqtt->loop();
        xSemaphoreGive(modemSemaphore);
    }
}

void MqttController::subscribe(MqttSubscribedTopic *topic)
{
    subscribedTopics.push_back(topic);
}

void MqttController::publish(const char *const topic, const char *const msg, const bool retain)
{
    if (xSemaphoreTake(modemSemaphore, (TickType_t)10) == pdTRUE)
    {
        mqtt->publish(topic, msg, retain);
        xSemaphoreGive(modemSemaphore);
    }
}

void MqttController::publish(const char *const topic, const uint8_t *const msg, uint32_t len, const bool retain)
{
    if (xSemaphoreTake(modemSemaphore, (TickType_t)10) == pdTRUE)
    {
        mqtt->publish(topic, msg, len, retain);
        xSemaphoreGive(modemSemaphore);
    }
}

void MqttController::messageCallback(char *topic, uint8_t *message, unsigned int messageLength)
{
    for (auto &subscribedTopic : subscribedTopics)
    {
        // todo optimisation
        if (strlen(subscribedTopic->topic) <= strlen(topic))
        {
            if (strncmp(subscribedTopic->topic, topic, strlen(subscribedTopic->topic)))
            {
            }
        }
    }
}
