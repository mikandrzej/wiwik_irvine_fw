#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <Arduino.h>
#include <EgTinyGsm.h>
#include <GpsData.h>
#include <SimpleIntervalTimer.h>
#include <ModemStatusData.h>

extern EgTinyGsm modem;

using MqttCallback = std::function<void(char *, uint8_t *, unsigned int)>;

enum class ModemManagementState
{
    POWER_OFF,
    MODEM_POWERING_ON,
    MODEM_SLEEP_ON,
    MODEM_SLEEP_OFF,
    GPRS_CONNECTING,
    GPRS_DISCONNECTING,
    MQTT_CONNECTING,
    GPS_POWERING_ON,
    GPS_POWERING_OFF,
    IDLE,
    MODEM_ENABLE,
};

class MqttSubscribedTopic
{
public:
    MqttSubscribedTopic(char *topic, MqttCallback callback);
    char *topic;
    size_t topicLength;
    MqttCallback callback;
};

#define MQTT_TOPIC_MAX_LEN 100u
#define MQTT_MSG_MAX_LEN 300u

struct MqttTxItem
{
    char topic[MQTT_TOPIC_MAX_LEN] = {0};
    char msg[MQTT_MSG_MAX_LEN] = {0};
    bool retain = false;
};

class ModemManagement
{
public:
    ModemManagement();
    bool begin();
    void loop();
    void subscribe(MqttSubscribedTopic *topic);

    bool mqttPublish(MqttTxItem &txItem);

    GpsData &getLastGpsData();
    GpsData &getLastValidGpsData();

private:
    void checkGpsInterval();
    void checkModemInfoInterval();
    void tryToSendMqttData();

    void mqttMessageCallback(char *topic, uint8_t *message, unsigned int messageLength);

    bool parseGpsData(const String &data);

    String getNextSubstring(const String &input, char separator, int *iterator);

    ModemManagementState state = ModemManagementState::POWER_OFF;

    ModemStatusData modemStatus;

    bool modemPowerOnRequest = true;
    bool modemConnectRequest = true;
    bool gpsEnabledRequest = true;

    bool gpsEnabled = false;
    bool modemPoweredOn = false;
    bool modemConnected = false;
    bool gprsConnected = false;
    bool mqttConnected = false;

    SimpleIntervalTimer gpsDataInterval = {1000u};
    SimpleIntervalTimer modemDataInterval = {1000u};

    static SemaphoreHandle_t gpsDataMutex;
    GpsData lastGpsData = {};
    GpsData lastValidGpsData = {};

    std::vector<MqttSubscribedTopic *> subscribedTopics;
};

extern ModemManagement modemManagement;

extern QueueHandle_t modemGpsRxQueue;