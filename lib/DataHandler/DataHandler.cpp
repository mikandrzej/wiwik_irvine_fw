#include "DataHandler.h"
#include <MqttController.h>
#include <Device.h>
#include <ArduinoJson.h>
#include <IrvineConfiguration.h>
#include <stdint.h>

const char MODULE[] = "DATA_HDL";

void DataHandler::handleJaaleeTemperatureData(const uint8_t bluetoothDeviceId, const float temperature, const float humidity, const float battery)
{
    uint64_t unixTimestamp = device.getUnixTimestamp();

    logger.logPrintF(LogSeverity::DEBUG, MODULE, "Unix timestamp: %llu", unixTimestamp);

    DynamicJsonDocument doc(100);
    doc["temp"] = temperature;
    doc["hum"] = humidity;
    doc["bat"] = battery;
    doc["t"] = unixTimestamp;

    char topic[100];
    char msg[100];
    char sensor[40];
    sprintf(sensor, "jaalee_%02x%02x%02x%02x%02x%02x",
            irvineConfiguration.bluetooth.devices[bluetoothDeviceId].macAddress[0],
            irvineConfiguration.bluetooth.devices[bluetoothDeviceId].macAddress[1],
            irvineConfiguration.bluetooth.devices[bluetoothDeviceId].macAddress[2],
            irvineConfiguration.bluetooth.devices[bluetoothDeviceId].macAddress[3],
            irvineConfiguration.bluetooth.devices[bluetoothDeviceId].macAddress[4],
            irvineConfiguration.bluetooth.devices[bluetoothDeviceId].macAddress[5]);
    sprintf(topic, "irvine/measures/%s/ble/%s", irvineConfiguration.device.deviceId, sensor);

    serializeJson(doc, msg, sizeof(msg));
    mqttController.publish(topic, msg, true);
}

void DataHandler::handleGpsData(const float longitude, const float latitude, const float altitude, const float speed, const uint64_t unixTimestamp, const uint8_t satellites, const float precision)
{
    DynamicJsonDocument doc(200);
    doc["gt"] = unixTimestamp; // gps timestamp
    doc["lon"] = longitude;
    doc["lat"] = latitude;
    doc["alt"] = altitude;
    doc["v"] = speed;
    doc["sat"] = satellites;
    doc["p"] = precision;
    doc["t"] = device.getUnixTimestamp();

    char topic[50];
    char msg[100];
    sprintf(topic, "irvine/measures/%s/gps/", irvineConfiguration.device.deviceId);

    serializeJson(doc, msg, sizeof(msg));
    mqttController.publish(topic, msg, true);
}
