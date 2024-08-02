#include "ServerConfiguration.h"
#include <string.h>
#include "ConfigurationHelpers.h"

bool ServerConfiguration::setMqttHost(const char *const value)
{
    return ConfigurationHelpers::copyString(value, mqttHost, sizeof(mqttHost));
}

bool ServerConfiguration::setMqttPort(const uint16_t value)
{
    mqttPort = value;
    return true;
}

bool ServerConfiguration::setMqttUsername(const char *const value)
{
    return ConfigurationHelpers::copyString(value, mqttUsername, sizeof(mqttUsername));
}

bool ServerConfiguration::setMqttPassword(const char *const value)
{
    return ConfigurationHelpers::copyString(value, mqttPassword, sizeof(mqttPassword));
}
