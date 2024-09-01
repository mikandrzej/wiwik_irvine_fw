#pragma once

#include <MqttController.h>

class Service
{
public:
    void begin();

    void mqttSetConfigMessageReceived(char *topic, uint8_t *message, unsigned int length);

private:
    MqttSubscribedTopic *mqttSubTopicSetConfig;
    char mqttTopicSetConfig[50];
};

extern Service service;