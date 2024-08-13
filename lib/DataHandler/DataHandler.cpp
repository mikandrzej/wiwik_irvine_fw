#include "DataHandler.h"
#include <MqttController.h>
#include <Device.h>
#include <IrvineConfiguration.h>
#include <stdint.h>
#include <DataLogger.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

const char MODULE[] = "DATA_HDL";

void DataHandler::handleJaaleeTemperatureData(const JaaleeData &data)
{
    logger.logPrintF(LogSeverity::DEBUG, MODULE, "Unix timestamp: %llu", data.unixTimestamp);

    char sensor[40];
    sprintf(sensor, "jaalee_%02x%02x%02x%02x%02x%02x",
            irvineConfiguration.bluetooth.devices[data.bleConfigId].macAddress[0],
            irvineConfiguration.bluetooth.devices[data.bleConfigId].macAddress[1],
            irvineConfiguration.bluetooth.devices[data.bleConfigId].macAddress[2],
            irvineConfiguration.bluetooth.devices[data.bleConfigId].macAddress[3],
            irvineConfiguration.bluetooth.devices[data.bleConfigId].macAddress[4],
            irvineConfiguration.bluetooth.devices[data.bleConfigId].macAddress[5]);

    char topic[100];
    sprintf(topic, "irvine/%s/measures/ble/%s", irvineConfiguration.device.deviceId, sensor);

    char msg[100];
    sprintf(msg, R"({"temp":%.2f,"hum":%.2f,"bat":%.0f,"rssi":%d,"t":%llu})", data.temperature, data.humidity, data.battery, data.rssi, data.unixTimestamp);
    mqttController.publish(topic, msg);

    DataLoggerQueueItem logItem{
        .data = new JaaleeData(data),
    };
    xQueueSend(dataLoggerQueue, &logItem, 0);
}

void DataHandler::handleM52PASTemperatureData(const M52PASData &data)
{
    logger.logPrintF(LogSeverity::DEBUG, MODULE, "Unix timestamp: %llu", data.unixTimestamp);

    char sensor[40];
    sprintf(sensor, "m52pas_%02x%02x%02x%02x%02x%02x",
            irvineConfiguration.bluetooth.devices[data.bleConfigId].macAddress[0],
            irvineConfiguration.bluetooth.devices[data.bleConfigId].macAddress[1],
            irvineConfiguration.bluetooth.devices[data.bleConfigId].macAddress[2],
            irvineConfiguration.bluetooth.devices[data.bleConfigId].macAddress[3],
            irvineConfiguration.bluetooth.devices[data.bleConfigId].macAddress[4],
            irvineConfiguration.bluetooth.devices[data.bleConfigId].macAddress[5]);

    char topic[100];
    sprintf(topic, "irvine/%s/measures/ble/%s", irvineConfiguration.device.deviceId, sensor);

    char msg[100];
    sprintf(msg, R"({"temp":%.2f,"hum":%.2f,"bat":%.0f,"rssi":%d,"t":%llu})", data.temperature, data.humidity, data.battery, data.rssi, data.unixTimestamp);
    mqttController.publish(topic, msg);

    DataLoggerQueueItem logItem{
        .data = new M52PASData(data),
    };
    xQueueSend(dataLoggerQueue, &logItem, 0);
}

void DataHandler::handleGpsData(const GpsData &gpsData)
{
    char topic[50];
    sprintf(topic, "irvine/%s/measures/gps", irvineConfiguration.device.deviceId);

    char msg[500];
    sprintf(msg, R"({"gt":%llu,"lng":%.5f,"lat":%.5f,"alt":%.1f,"spd":%.2f,"sat":%d,"t":%llu})",
            gpsData.gpsUnixTimestamp,
            gpsData.longitude,
            gpsData.latitude,
            gpsData.altitude,
            gpsData.speed,
            gpsData.satellites,
            gpsData.DataLoggable::unixTimestamp);

    mqttController.publish(topic, msg);

    DataLoggerQueueItem logItem{
        .data = new GpsData(gpsData)};
    xQueueSend(dataLoggerQueue, &logItem, 0);
}
