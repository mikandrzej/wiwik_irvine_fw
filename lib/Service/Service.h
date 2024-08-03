#pragma once

#include <MqttController.h>

class Service
{
public:
    void begin();

    void mqttMessageReceived(char *topic, uint8_t *message, unsigned int length);

private:
    MqttSubscribedTopic *mqttSubscribedTopic;
};

extern Service service;