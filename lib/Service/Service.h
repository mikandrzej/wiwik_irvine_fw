#pragma once

#include <ModemManagement.h>

class Service
{
public:
    void begin();
    

    void mqttSetConfigMessageReceived(char *topic, uint8_t *message, unsigned int length);
    void mqttUpdateMessageReceived(char *topic, uint8_t *message, unsigned int length);
    void mqttCalibrationMessageReceived(char *topic, uint8_t *message, unsigned int length);

    bool batteryCalibration(const float reference_voltage);

private:
    MqttSubscribedTopic *mqttSubTopicSetConfig;
    MqttSubscribedTopic *mqttSubTopicUpdate;
    MqttSubscribedTopic *mqttSubTopicCalibration;
    char mqttTopicSetConfig[50];
    char mqttTopicUpdate[50];
    char mqttTopicCalibration[50];
};

extern Service service;