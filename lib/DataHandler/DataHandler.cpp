#include "DataHandler.h"
#include <MqttController.h>
#include <Device.h>
#include <ArduinoJson.h>
#include <IrvineConfiguration.h>
#include <stdint.h>
#include <DataLogger.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

const char MODULE[] = "DATA_HDL";

void DataHandler::handleJaaleeTemperatureData(const JaaleeData &data)
{
    logger.logPrintF(LogSeverity::DEBUG, MODULE, "Unix timestamp: %llu", data.unixTimestamp);

    DynamicJsonDocument doc(100);
    doc["temp"] = data.temperature;
    doc["hum"] = data.humidity;
    doc["bat"] = data.battery;
    doc["rssi"] = data.rssi;
    doc["t"] = data.unixTimestamp;

    char topic[100];
    char msg[100];
    char sensor[40];
    sprintf(sensor, "jaalee_%02x%02x%02x%02x%02x%02x",
            irvineConfiguration.bluetooth.devices[data.bleConfigId].macAddress[0],
            irvineConfiguration.bluetooth.devices[data.bleConfigId].macAddress[1],
            irvineConfiguration.bluetooth.devices[data.bleConfigId].macAddress[2],
            irvineConfiguration.bluetooth.devices[data.bleConfigId].macAddress[3],
            irvineConfiguration.bluetooth.devices[data.bleConfigId].macAddress[4],
            irvineConfiguration.bluetooth.devices[data.bleConfigId].macAddress[5]);
    sprintf(topic, "irvine/%s/measures/ble/%s", irvineConfiguration.device.deviceId, sensor);

    serializeJson(doc, msg, sizeof(msg));
    mqttController.publish(topic, msg);

    DataLoggerQueueItem logItem{
        .data = new JaaleeData(data),
    };
    xQueueSend(dataLoggerQueue, &logItem, 0);
}

void DataHandler::handleGpsData(const GpsData &gpsData)
{
    DynamicJsonDocument doc(500);
    doc["gt"] = gpsData.unixTimestamp; // gps timestamp
    doc["lng"] = gpsData.longitude;
    doc["lat"] = gpsData.latitude;
    doc["alt"] = gpsData.altitude;
    doc["spd"] = gpsData.speed;
    doc["sat"] = gpsData.satellites;
    doc["t"] = gpsData.DataLoggable::unixTimestamp;

    char topic[50];
    char msg[500];
    sprintf(topic, "irvine/%s/measures/gps", irvineConfiguration.device.deviceId);

    serializeJson(doc, msg, sizeof(msg));
    mqttController.publish(topic, msg);

    DataLoggerQueueItem logItem{
        .data = new GpsData(gpsData)
    };
    xQueueSend(dataLoggerQueue, &logItem, 0);
}
