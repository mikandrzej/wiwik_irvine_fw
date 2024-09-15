#pragma once

#include <ModemManagement.h>

class Service
{
public:
    void begin();

    void mqttSetConfigMessageReceived(char *topic, uint8_t *message, unsigned int length);
    void mqttUpdateMessageReceived(char *topic, uint8_t *message, unsigned int length);

private:
    MqttSubscribedTopic *mqttSubTopicSetConfig;
    MqttSubscribedTopic *mqttSubTopicUpdate;
    char mqttTopicSetConfig[50];
    char mqttTopicUpdate[50];
};

extern Service service;