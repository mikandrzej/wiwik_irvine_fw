#include <ModemManagement.h>
#include <Logger.h>
#include <IrvineConfiguration.h>
#include <TimeLib.h>
#include <Device.h>
#include <HwConfiguration.h>
#include <PubSubClient.h>
#include <Queues.h>
#include "TimeCounter.h"

const char MODULE[] = "MODEM_MNG";

#define SerialAT Serial1
#define MODEM_UART_BAUD 115200

ModemManagement modemManagement;
EgTinyGsm modem(SerialAT);
TinyGsmClient mqttClient(modem, 0);
TinyGsmClient updateClient(modem, 1);
HttpClient httpClient(updateClient, "firmware.7frost.com");
PubSubClient mqtt(mqttClient);

SemaphoreHandle_t ModemManagement::gpsDataMutex = xSemaphoreCreateMutex();

ModemManagement::ModemManagement()
{
}

bool ModemManagement::begin()
{
    SerialAT.setRxBufferSize(6144);

    pinMode(BOARD_MODEM_RESET_PIN, OUTPUT);
    pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);
    digitalWrite(BOARD_MODEM_RESET_PIN, HIGH);
    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);

    mqtt.setCallback([this](char *topic, uint8_t *msg, unsigned int len)
                     { this->mqttMessageCallback(topic, msg, len); });
    return true;
}

void ModemManagement::loop()
{
    bool success;

    switch (state)
    {
    case ModemManagementState::POWER_OFF:
        if (modemPowerOnRequest)
        {
            state = ModemManagementState::MODEM_POWERING_ON1;
        }
        break;

    case ModemManagementState::MODEM_POWERING_ON1:

        logger.logPrintF(LogSeverity::DEBUG, MODULE, "Modem reset");
        // digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
        // digitalWrite(BOARD_MODEM_RESET_PIN, LOW);
        // vTaskDelay(pdMS_TO_TICKS(100));
        digitalWrite(BOARD_MODEM_RESET_PIN, HIGH);
        sm_timestamp = millis();
        state = ModemManagementState::MODEM_POWERING_ON2;
        break;

    case ModemManagementState::MODEM_POWERING_ON2:
        if ((millis() - sm_timestamp) >= 2000)
        {
            digitalWrite(BOARD_MODEM_RESET_PIN, LOW);
            digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
            logger.logPrintF(LogSeverity::DEBUG, MODULE, "Modem power up");
            sm_timestamp = millis();
            state = ModemManagementState::MODEM_POWERING_ON3;
        }
        break;

    case ModemManagementState::MODEM_POWERING_ON3:
        if ((millis() - sm_timestamp) >= 500)
        {
            digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
            sm_timestamp = millis();
            state = ModemManagementState::MODEM_POWERING_ON4;
            logger.logPrintF(LogSeverity::DEBUG, MODULE, "Modem power up delay");
        }
        break;

    case ModemManagementState::MODEM_POWERING_ON4:
        if ((millis() - sm_timestamp) >= 5000)
        {
            state = ModemManagementState::MODEM_POWERING_ON5;
        }
        break;

    case ModemManagementState::MODEM_POWERING_ON5:
    {
        success = true;
        SerialAT.end();
        SerialAT.begin(115200, SERIAL_8N1, BOARD_MODEM_RXD_PIN, BOARD_MODEM_TXD_PIN);

        // todo obtain modem data
        if (!modem.init())
        {
            logger.logPrintF(LogSeverity::ERROR, MODULE, "Modem initialization failed");
            success = false;
        }

        if (success)
        {
            modem.setBaud(921600);
            SerialAT.end();
            SerialAT.begin(921600, SERIAL_8N1, BOARD_MODEM_RXD_PIN, BOARD_MODEM_TXD_PIN);

            logger.logPrintF(LogSeverity::DEBUG, MODULE, "Modem initialized");

            modem.setNetworkMode(38);

            modemStatus.modemName = modem.getModemName();
            logger.logPrintF(LogSeverity::INFO, MODULE, "Modem name: %s", modemStatus.modemName.c_str());
            modemStatus.modemModel = modem.getModemModel();
            logger.logPrintF(LogSeverity::INFO, MODULE, "Modem model: %s", modemStatus.modemModel.c_str());
            modemStatus.modemImei = modem.getIMEI();
            logger.logPrintF(LogSeverity::INFO, MODULE, "Modem IMEI: %s", modemStatus.modemImei.c_str());

            state = ModemManagementState::MODEM_SIM_UNLOCK;
        }
    }

    break;

    case ModemManagementState::MODEM_SIM_UNLOCK:
    {
        success = true;
        SimStatus sim_status = modem.getSimStatus(10000);
        if (sim_status == SIM_LOCKED)
        {
            modemStatus.pinEnabled = true;
            if (modem.simUnlock(irvineConfiguration.modem.pin))
            {
                sim_status = modem.getSimStatus(10000);
                if (sim_status == SIM_READY)
                {
                    modemStatus.simReady = true;
                    logger.logPrintF(LogSeverity::INFO, MODULE, "SIM unlocked");
                }
            }
            else
            {
                logger.logPrintF(LogSeverity::ERROR, MODULE, "SIM unlock error");
                success = false;
            }
        }
        else if (sim_status == SIM_READY)
        {
            modemStatus.pinEnabled = false;
            modemStatus.simReady = true;
            logger.logPrintF(LogSeverity::INFO, MODULE, "SIM lock disabled");
        }
        else
        {
            logger.logPrintF(LogSeverity::ERROR, MODULE, "SIM error");
            success = false;
        }

        if (success)
        {
            vTaskDelay(pdMS_TO_TICKS(3000));

            modemStatus.simCcid = modem.getSimCCID();
            modemStatus.simImsi = modem.getIMSI();
            logger.logPrintF(LogSeverity::INFO, MODULE, "SIM CCID: %s, SIM IMSI: %s", modemStatus.simCcid.c_str(), modemStatus.simImsi.c_str());

            modemStatus.modemPoweredOn = true;
        }
        state = ModemManagementState::IDLE;
        break;
    }

    case ModemManagementState::MODEM_SLEEP_ON:
        // modem.poweroff();
        // modem.radioOff();
        modem.sleepEnable(1);
        modemStatus.gpsEnabled = false;
        modemStatus.modemPoweredOn = false;
        modemStatus.gprsConnected = false;
        modemStatus.mqttConnected = false;

        // esp_deep_sleep_start();
        // esp_sleep_enable_timer_wakeup(500);
        // esp_light_sleep_start();

        state = ModemManagementState::IDLE;
        break;

    case ModemManagementState::MODEM_SLEEP_OFF:
        // modem.poweroff();
        // modem.radioOff();
        modem.sleepEnable(0);
        modemStatus.modemPoweredOn = true;
        modemStatus.gpsEnabled = true;
        // gprsConnected = false;
        // mqttConnected = false;

        // esp_deep_sleep_start();
        // esp_sleep_enable_timer_wakeup(500);
        // esp_light_sleep_start();

        state = ModemManagementState::IDLE;
        break;

    case ModemManagementState::GPRS_CONNECTING:
        if (modem.isNetworkConnected())
        {

            if (modem.gprsConnect(irvineConfiguration.modem.apn,
                                  irvineConfiguration.modem.apnUsername,
                                  irvineConfiguration.modem.apnPassword))
            {
                logger.logPrintF(LogSeverity::INFO, MODULE, "APN %s:%s/%s connected",
                                 irvineConfiguration.modem.apn,
                                 irvineConfiguration.modem.apnUsername,
                                 irvineConfiguration.modem.apnPassword);
            }
        }
        state = ModemManagementState::IDLE;
        break;

    case ModemManagementState::GPRS_DISCONNECTING:
        modem.gprsDisconnect();
        state = ModemManagementState::IDLE;
        break;

    case ModemManagementState::MQTT_CONNECTING:
        if (irvineConfiguration.server.mqttHost[0u] == '\0')
        {
            success = false;
        }
        if (irvineConfiguration.server.mqttUsername[0u] != '\0')
        {
            logger.logPrintF(LogSeverity::INFO, MODULE, "Connecting to MQTT %s:%d with %s/%s as %s",
                             irvineConfiguration.server.mqttHost,
                             irvineConfiguration.server.mqttPort,
                             irvineConfiguration.server.mqttUsername,
                             irvineConfiguration.server.mqttPassword,
                             irvineConfiguration.device.deviceId);
            mqtt.setServer(irvineConfiguration.server.mqttHost, irvineConfiguration.server.mqttPort);
            success = mqtt.connect(irvineConfiguration.device.deviceId,
                                   irvineConfiguration.server.mqttUsername,
                                   irvineConfiguration.server.mqttPassword);

            mqtt.setKeepAlive(60);
        }
        else
        {
            logger.logPrintF(LogSeverity::WARNING, MODULE, "Connecting to MQTT %s:%d without credetials as %s",
                             irvineConfiguration.server.mqttHost,
                             irvineConfiguration.server.mqttPort,
                             irvineConfiguration.device.deviceId);
            mqtt.setServer(irvineConfiguration.server.mqttHost, irvineConfiguration.server.mqttPort);
            success = mqtt.connect(irvineConfiguration.device.deviceId);
        }

        if (success)
        {
            logger.logPrintF(LogSeverity::INFO, MODULE, "Connected to MQTT");

            for (auto &subscribedTopic : subscribedTopics)
            {
                success = mqtt.subscribe(subscribedTopic->topic);
                if (success)
                {
                    logger.logPrintF(LogSeverity::INFO, MODULE, "Succesfully subscribed topic %s", subscribedTopic->topic);
                }
                else
                {
                    logger.logPrintF(LogSeverity::ERROR, MODULE, "Failed to subscribe topic %s", subscribedTopic->topic);
                    break;
                }
            }
        }
        else
        {
            logger.logPrintF(LogSeverity::INFO, MODULE, "Connecting to MQTT failed");
        }
        state = ModemManagementState::IDLE;
        break;

    case ModemManagementState::GPS_POWERING_ON1:
        logger.logPrintF(LogSeverity::INFO, MODULE, "GPS initialization start...");
        modem.sendAT("+CGNSSPWR=0");
        state = ModemManagementState::GPS_POWERING_ON2;
        sm_timestamp = millis();
        break;

    case ModemManagementState::GPS_POWERING_ON2:
    {
        int8_t response = modem.waitResponse(0u);
        if (response == 1)
        {
            logger.logPrintF(LogSeverity::DEBUG, MODULE, "GPS initialization 1st stage done");
            modem.sendAT("+CGNSSPWR=1");
            state = ModemManagementState::GPS_POWERING_ON3;
            sm_timestamp = millis();
        }
        else if (!response && ((millis() - sm_timestamp) >= 1000))
        {
            logger.logPrintF(LogSeverity::ERROR, MODULE, "GPS initialization 1st stage error");
            state = ModemManagementState::IDLE;
        }
        break;
    }

    case ModemManagementState::GPS_POWERING_ON3:
    {
        int8_t response = modem.waitResponse(0u);
        if (response == 1)
        {
            logger.logPrintF(LogSeverity::DEBUG, MODULE, "GPS initialization 2nd stage done");
            state = ModemManagementState::GPS_POWERING_ON4;
            sm_timestamp = millis();
        }
        else if (!response && ((millis() - sm_timestamp) >= 1000))
        {
            logger.logPrintF(LogSeverity::ERROR, MODULE, "GPS initialization 2nd stage error");
            state = ModemManagementState::IDLE;
        }
        break;
    }

    case ModemManagementState::GPS_POWERING_ON4:
    {
        int8_t response = modem.waitResponse(0u, "+CGNSSPWR: READY!");
        if (response == 1)
        {
            logger.logPrintF(LogSeverity::DEBUG, MODULE, "GPS initialization 3rd stage done");
            modem.sendAT("+CGNSSMODE=3");
            sm_timestamp = millis();
            state = ModemManagementState::GPS_POWERING_ON5;
        }
        else if (!response && ((millis() - sm_timestamp) >= 20000))
        {
            logger.logPrintF(LogSeverity::ERROR, MODULE, "GPS initialization 3rd stage error");
            state = ModemManagementState::IDLE;
        }
        break;
    }
    case ModemManagementState::GPS_POWERING_ON5:
    {
        int8_t response = modem.waitResponse(0u);
        if (response == 1)
        {
            logger.logPrintF(LogSeverity::INFO, MODULE, "GPS initialization done");
            modemStatus.gpsEnabled = true;
            state = ModemManagementState::IDLE;
        }
        else if (!response && ((millis() - sm_timestamp) >= 20000))
        {
            logger.logPrintF(LogSeverity::ERROR, MODULE, "GPS initialization 4th stage error");
            state = ModemManagementState::IDLE;
        }
        break;
    }

    case ModemManagementState::IDLE:

        if (modemStatus.modemPoweredOn)
        {
            if (!modemPowerOnRequest)
            {
                logger.logPrintF(LogSeverity::INFO, MODULE, "Modem power OFF request");
                state = ModemManagementState::MODEM_SLEEP_ON;
                break;
            }

            modemStatus.gprsConnected = modem.isGprsConnected();
            modemStatus.mqttConnected = mqtt.loop();

            tryToSendMqttData();

            // logger.logPrintF(LogSeverity::DEBUG, MODULE, "GPRS: %d, MQTT: %d, GPS: %d", gprsConnected, mqttConnected, gpsEnabled);

            checkModemInfoInterval();
            checkGpsInterval();

            if (modemConnectRequest)
            {
                if (!modemStatus.gprsConnected)
                {
                    logger.logPrintF(LogSeverity::INFO, MODULE, "Trying to connect GPRS");
                    state = ModemManagementState::GPRS_CONNECTING;
                    break;
                }
                if (!modemStatus.mqttConnected)
                {
                    logger.logPrintF(LogSeverity::INFO, MODULE, "Trying to connect MQTT");
                    state = ModemManagementState::MQTT_CONNECTING;
                    break;
                }
            }

            if (gpsEnabledRequest)
            {
                if (!modemStatus.gpsEnabled)
                {
                    logger.logPrintF(LogSeverity::INFO, MODULE, "GPS enable request");
                    state = ModemManagementState::GPS_POWERING_ON1;
                    break;
                }
            }
            else
            {
                if (modemStatus.gpsEnabled)
                {
                    logger.logPrintF(LogSeverity::INFO, MODULE, "GPS disable request");
                    state = ModemManagementState::GPS_POWERING_OFF;
                    break;
                }
            }
        }
        else
        {
            if (modemPowerOnRequest)
            {
                state = ModemManagementState::MODEM_POWERING_ON1;
            }
        }
    }
}

void ModemManagement::subscribe(MqttSubscribedTopic *topic)
{
    subscribedTopics.push_back(topic);
}

bool ModemManagement::mqttPublish(MqttTxItem &txItem)
{
    return pdTRUE == xQueueSend(queues.modemMqttTxQueue, &txItem, 0);
}

ModemStatusData &ModemManagement::getModemStatusData()
{
    return modemStatus;
}

void ModemManagement::checkGpsInterval()
{
    if (gpsDataInterval.check())
    {
        parseGpsData(modem.getGPSraw());

        xQueueSend(queues.modemGpsRxQueue, &this->lastGpsData, 0);
    }
}

void ModemManagement::checkModemInfoInterval()
{
    if (modemDataInterval.check())
    {
        modemStatus.signal = modem.getSignalQuality();
        // logger.logPrintF(LogSeverity::DEBUG, MODULE, "GSM location: %s", modem.getGsmLocation().c_str());
        // logger.logPrintF(LogSeverity::DEBUG, MODULE, "Modem IMEI: %s", modem.getIMEI().c_str());
        // logger.logPrintF(LogSeverity::DEBUG, MODULE, "GSM network mode %d", modem.getNetworkMode());
        // logger.logPrintF(LogSeverity::DEBUG, MODULE, "GSM network modes %s", modem.getNetworkModes().c_str());
        // logger.logPrintF(LogSeverity::DEBUG, MODULE, "GSM operator %s", modem.getOperator().c_str());
        // logger.logPrintF(LogSeverity::DEBUG, MODULE, "GSM regStatus %d", modem.getRegistrationStatus());
        // logger.logPrintF(LogSeverity::DEBUG, MODULE, "Modem temperature %.1f", modem.getTemperature());
        // logger.logPrintF(LogSeverity::DEBUG, MODULE, "Modem voltage %u", modem.getBattVoltage());

        int yearNetwork;
        int monthNetwork;
        int dayNetwork;
        int hourNetwork;
        int minuteNetwork;
        int secondNetwork;
        float timezoneNetwork;

        if (true == modem.getNetworkTime(
                        &yearNetwork,
                        &monthNetwork,
                        &dayNetwork,
                        &hourNetwork,
                        &minuteNetwork,
                        &secondNetwork,
                        &timezoneNetwork))
        {

            tmElements_t tm;

            tm.Day = dayNetwork;
            tm.Month = monthNetwork;
            tm.Year = yearNetwork - 1970;
            tm.Hour = hourNetwork;
            tm.Minute = minuteNetwork;
            tm.Second = secondNetwork;

            uint64_t unixTimestamp = makeTime(tm);

            // offset timezone
            unixTimestamp -= (int)timezoneNetwork * 60 * 60;
            device.updateGsmTime(unixTimestamp);

            logger.logPrintF(LogSeverity::DEBUG, MODULE, "time %d-%d-%d %d:%d:%d timezone %f unix:%llu",
                             yearNetwork,
                             monthNetwork,
                             dayNetwork,
                             hourNetwork,
                             minuteNetwork,
                             secondNetwork,
                             timezoneNetwork,
                             unixTimestamp);
        }
        else
        {
            logger.logPrintF(LogSeverity::ERROR, MODULE, "Failed to get timestamp from GSM");
        }

        modem.sendAT(GF("+CPSI?"));
        if (modem.waitResponse(GF("+CPSI: ")) == 1)
        {
            modemStatus.gsmNetworkType = modem.stream.readStringUntil(',');
            if (modemStatus.gsmNetworkType.equals("LTE"))
            {
                modem.stream.readStringUntil(',');                            // Operation Mode
                modemStatus.gsmOperator = modem.stream.readStringUntil(',');  // MCC
                modem.stream.readStringUntil(',');                            // MNC
                modem.stream.readStringUntil(',');                            // TAC
                modem.stream.readStringUntil(',');                            // SCellID
                modem.stream.readStringUntil(',');                            // PCEllID
                modemStatus.gsmFrequency = modem.stream.readStringUntil(','); // FrequencyBand
                modem.stream.readStringUntil('\n');

                modem.waitResponse();
            }
            else if (modemStatus.gsmNetworkType.equals("GSM"))
            {
                modem.stream.readStringUntil(',');                           // Operation Mode
                modemStatus.gsmOperator = modem.stream.readStringUntil(','); // MCC
                modemStatus.gsmFrequency = "-";
                modem.stream.readStringUntil('\n');

                modem.waitResponse();
            }

            logger.logPrintF(LogSeverity::DEBUG, MODULE, "Network type: %s", modemStatus.gsmNetworkType.c_str());
            logger.logPrintF(LogSeverity::DEBUG, MODULE, "Operator: %s", modemStatus.gsmOperator.c_str());
            logger.logPrintF(LogSeverity::DEBUG, MODULE, "Frequency: %s", modemStatus.gsmFrequency.c_str());
        }
    }
}

void ModemManagement::tryToSendMqttData()
{
    if (modemStatus.mqttConnected)
    {
        MqttTxItem item;
        while (pdTRUE == xQueuePeek(queues.modemMqttTxQueue, &item, 0))
        {
            if (mqtt.publish(item.topic, item.msg, item.retain))
            {
                logger.logPrintF(LogSeverity::DEBUG, MODULE, "Published MQTT data to topic: %s", item.topic);
                xQueueReceive(queues.modemMqttTxQueue, &item, 0);
            }
            else
            {
                break;
            }
        }
    }
}

void ModemManagement::mqttMessageCallback(char *topic, uint8_t *message, unsigned int messageLength)
{
    message[messageLength] = '\0';
    logger.logPrintF(LogSeverity::INFO, MODULE, "Message callback triggered for topic %s with msg: %s", topic, message);
    for (auto &subscribedTopic : subscribedTopics)
    {
        const char *subscribedTopicStr = subscribedTopic->topic;
        size_t subscribedTopicLen = strlen(subscribedTopicStr);
        size_t topicLen = strlen(topic);

        if (subscribedTopicLen <= topicLen)
        {
            // Sprawdź, czy subskrybowany topic zawiera '#' na końcu
            if (subscribedTopicStr[subscribedTopicLen - 1] == '#')
            {
                // Sprawdź, czy początek topicu zgadza się z subskrybowanym topicem bez '#'
                if (strncmp(subscribedTopicStr, topic, subscribedTopicLen - 1) == 0 &&
                    (topic[subscribedTopicLen - 1] == '/' || topic[subscribedTopicLen - 1] == '\0'))
                {
                    logger.logPrintF(LogSeverity::INFO, MODULE, "Message callback triggered for wildcard subscription");
                    subscribedTopic->callback(topic, message, messageLength);
                }
            }
            else
            {
                // Standardowe porównanie topiców bez symbolu '#'
                if (strncmp(subscribedTopicStr, topic, subscribedTopicLen) == 0 &&
                    (topic[subscribedTopicLen] == '/' || topic[subscribedTopicLen] == '\0'))
                {
                    logger.logPrintF(LogSeverity::INFO, MODULE, "Message callback triggered for exact match subscription");
                    subscribedTopic->callback(topic, message, messageLength);
                }
            }
        }
    }
}

MqttSubscribedTopic::MqttSubscribedTopic(char *topic, MqttCallback callback)
{
    this->topic = topic;
    this->topicLength = strlen(topic);
    this->callback = callback;
}

bool ModemManagement::parseGpsData(const String &data)
{
    bool valid = false;

    uint8_t mode;
    uint8_t satellites;
    double latitude;
    double longitude;
    double altitude;
    double speed;
    uint64_t unixTimestamp;
    uint64_t gpsUnixTimestamp;

    if (data.length() > 20)
    {

        int iterator = 0;
        mode = (uint8_t)getNextSubstring(data, ',', &iterator).toInt();
        satellites = (uint8_t)getNextSubstring(data, ',', &iterator).toInt();

        (void)getNextSubstring(data, ',', &iterator);
        (void)getNextSubstring(data, ',', &iterator);
        (void)getNextSubstring(data, ',', &iterator);

        // format 1: +CGNSSINFO: 3,12,,  ,  ,52.3953667,N,16.7443104 ,E,111223,213028.00,107.7,0.992,,3.18
        // format 2: +CGNSSINFO: 3,14,,00,01,5223.72163,N,01644.66370,E,111223,213524.00,93.5 ,4.551,,

        auto latitudeStr = getNextSubstring(data, ',', &iterator);
        char ns = getNextSubstring(data, ',', &iterator)[0];
        auto longitudeStr = getNextSubstring(data, ',', &iterator);
        char ew = getNextSubstring(data, ',', &iterator)[0];

        if (latitudeStr[2] == '.')
        {
            latitude = latitudeStr.toDouble();
            longitude = longitudeStr.toDouble();
        }
        else
        {
            auto lat = latitudeStr.toDouble();
            latitude = (int)lat / 100;
            lat -= latitude * 100;
            latitude += lat / 60.0;

            auto lng = longitudeStr.toDouble();
            longitude = (int)lng / 100;
            lng -= longitude * 100;
            longitude += lng / 60.0;
        }

        if (ns != 'N')
            latitude *= -1.0;
        if (ew != 'E')
            longitude *= -1.0;

        String date = getNextSubstring(data, ',', &iterator);
        String time = getNextSubstring(data, ',', &iterator);

        tmElements_t tm;
        int day, month, year, hour, minute, second;
        sscanf(date.c_str(), "%2d%2d%2d", &day, &month, &year);
        sscanf(time.c_str(), "%2d%2d%2d", &hour, &minute, &second);

        tm.Day = day;
        tm.Month = month;
        tm.Year = CalendarYrToTm(2000 + year); // Dodajemy 2000, ponieważ rok podawany jest jako YY
        tm.Hour = hour;
        tm.Minute = minute;
        tm.Second = second;

        gpsUnixTimestamp = makeTime(tm);
        device.updateGpsTime(gpsUnixTimestamp);

        altitude = getNextSubstring(data, ',', &iterator).toDouble();
        double speed_knots = getNextSubstring(data, ',', &iterator).toDouble();
        speed = speed_knots * 1.852000;

        (void)getNextSubstring(data, ',', &iterator);
        (void)getNextSubstring(data, ',', &iterator);

        unixTimestamp = device.getUnixTimestamp();

        valid = true;
    }

    if (xSemaphoreTake(this->gpsDataMutex, portMAX_DELAY))
    {
        if (valid)
        {
            lastGpsData = GpsData(mode, satellites, latitude, longitude, altitude, speed, gpsUnixTimestamp, unixTimestamp);
            lastValidGpsData = lastGpsData;
        }
        else
            lastGpsData = GpsData();
        xSemaphoreGive(gpsDataMutex);
    }

    return valid;
}

String ModemManagement::getNextSubstring(const String &input, char separator, int *iterator)
{
    int start = *iterator;
    int end = *iterator;

    if (input[end] == separator)
    {
        (*iterator)++;
        return input.substring(start, end);
    }
    while (end < input.length())
    {
        if (input[end] == separator)
        {
            *iterator = end + 1;
            return input.substring(start, end);
        }
        end++;
    }
    *iterator = end;
    return input.substring(start, end);
}