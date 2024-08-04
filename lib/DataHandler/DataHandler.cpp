#include "DataHandler.h"
#include <MqttController.h>
#include <Device.h>
#include <ArduinoJson.h>
#include <IrvineConfiguration.h>
#include <stdint.h>

const char MODULE[] = "DATA_HDL";

void DataHandler::handleJaaleeTemperatureData(const uint8_t bluetoothDeviceId, const float temperature, const float humidity, const float battery, const int16_t rssi)
{
    uint64_t unixTimestamp = device.getUnixTimestamp();

    logger.logPrintF(LogSeverity::DEBUG, MODULE, "Unix timestamp: %llu", unixTimestamp);

    DynamicJsonDocument doc(100);
    doc["temp"] = temperature;
    doc["hum"] = humidity;
    doc["bat"] = battery;
    doc["rssi"] = rssi;
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
    sprintf(topic, "irvine/%s/measures/ble/%s", irvineConfiguration.device.deviceId, sensor);

    serializeJson(doc, msg, sizeof(msg));
    mqttController.publish(topic, msg);
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
    doc["t"] = device.getUnixTimestamp();

    char topic[50];
    char msg[500];
    sprintf(topic, "irvine/%s/measures/gps", irvineConfiguration.device.deviceId);

    serializeJson(doc, msg, sizeof(msg));
    mqttController.publish(topic, msg);
}
