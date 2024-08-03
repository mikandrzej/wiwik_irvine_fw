#pragma once

#include <stdint.h>
#include "EgTinyGsm.h"
#include <PubSubClient.h>

enum class MqttState
{
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    DISCONNECT
};

using MqttCallback = std::function<void(char *, uint8_t *, unsigned int)>;

class MqttSubscribedTopic
{
public:
    MqttSubscribedTopic(char *topic, MqttCallback callback);
    char *topic;
    size_t topicLength;
    MqttCallback callback;
};

class MqttController
{
public:
    MqttController(EgTinyGsm &modem);
    void begin();
    void loop();
    void subscribe(MqttSubscribedTopic *topic);
    void publish(const char *const topic, const char *const msg);
    void publish(const char *const topic, const uint8_t *const msg, uint32_t len);

private:
    void messageCallback(char *topic, uint8_t *message, unsigned int messageLength);

    TinyGsmClient client;
    PubSubClient mqtt;
    MqttState state = MqttState::DISCONNECTED;
    // todo shared pointer
    std::vector<MqttSubscribedTopic *> subscribedTopics;
};

extern MqttController mqttController;