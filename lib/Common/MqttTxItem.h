#pragma once

#define MQTT_TOPIC_MAX_LEN 100u
#define MQTT_MSG_MAX_LEN 300u

struct MqttTxItem
{
    char topic[MQTT_TOPIC_MAX_LEN] = {0};
    char msg[MQTT_MSG_MAX_LEN] = {0};
    bool retain = false;
};