#pragma once

#include <stdint.h>

class ServerConfiguration
{
public:
    char mqttHost[50u] = "";
    uint16_t mqttPort = 0u;
    char mqttUsername[50u] = "";
    char mqttPassword[50u] = "";

    char managementHost[50u] = "";
    uint16_t managementPort = 0u;
    char managementUsername[50u] = "";
    char managementPassword[50u] = "";

    bool setMqttHost(const char *const value);
    bool setMqttPort(const uint16_t value);
    bool setMqttUsername(const char *const value);
    bool setMqttPassword(const char *const value);
};