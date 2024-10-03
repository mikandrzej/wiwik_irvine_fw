#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <Arduino.h>
#include <EgTinyGsm.h>
#include <GpsData.h>
#include <SimpleIntervalTimer.h>
#include <ModemStatusData.h>
#include <MqttTxItem.h>
#include <ArduinoHttpClient.h>

extern EgTinyGsm modem;
extern TinyGsmClient updateClient;
extern HttpClient httpClient;

using MqttCallback = std::function<void(char *, uint8_t *, unsigned int)>;

enum class ModemManagementState
{
    POWER_OFF,
    MODEM_POWERING_ON1,
    MODEM_POWERING_ON2,
    MODEM_POWERING_ON3,
    MODEM_POWERING_ON4,
    MODEM_POWERING_ON5,
    MODEM_SIM_UNLOCK,
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
    ModemStatusData &getModemStatusData();
    

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

    SimpleIntervalTimer gpsDataInterval = {1000u};
    SimpleIntervalTimer modemDataInterval = {5000u};

    static SemaphoreHandle_t gpsDataMutex;
    GpsData lastGpsData = {};
    GpsData lastValidGpsData = {};

    std::vector<MqttSubscribedTopic *> subscribedTopics;
    uint32_t sm_timestamp;
};

extern ModemManagement modemManagement;
