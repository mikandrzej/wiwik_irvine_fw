#include "BluetoothM52PASTempSensor.h"
#include <Logger.h>
#include <IrvineConfiguration.h>
#include <DataHandler.h>
#include <Device.h>

const char MODULE[] = "M52PAS";

typedef struct
{
    uint8_t bleFlag[3u];
    uint32_t service;
    uint32_t service2;
    uint8_t device_id[6u];
    uint32_t timestamp;
    uint16_t rawTemperature;
    uint16_t rawHumidity;
    uint16_t batteryVoltage;
    uint16_t triggerCountdown;
} __attribute__((packed)) M52PASAdvertisedDataStruct;

BluetoothM52PASTempSensor::BluetoothM52PASTempSensor(const uint16_t configIndex) : AbstractBluetoothBleDevice(configIndex)
{
}

void BluetoothM52PASTempSensor::parseAdvertisedData(const uint8_t *const data, const uint16_t len, int16_t rssi)
{
    char txt[500];

    if (len != sizeof(M52PASAdvertisedDataStruct))
    {
        logger.logPrintF(LogSeverity::WARNING, MODULE, "Incorrect advertised data length: %d. Expected %d", len, sizeof(M52PASAdvertisedDataStruct));
        return;
    }

    const M52PASAdvertisedDataStruct *const structedData = (const M52PASAdvertisedDataStruct *)data;

    float temperature = ((float)structedData->rawTemperature / 65535.0f) * 175.72f - 46.85f;
    float humidity = (125.0f * (float)structedData->rawHumidity) / 65536.0f - 6.0f;
    float battery = ((float)structedData->batteryVoltage - 2000.0f) / 10.0f;
    battery = battery > 100.0f ? 100.0f : battery;
    battery = battery < 0.0f ? 0.0f : battery;

    logger.logPrintF(LogSeverity::DEBUG, MODULE, "sensor %d temp: %.2f hum: %.2f bat: %.0f%% rssi: %d", configIndex, temperature, humidity, battery, rssi);

    bool doNotify = false;

    uint32_t currTimestamp = millis();

    if (firstRead)
    {
        doNotify = true;
        firstRead = false;
    }
    else if ((currTimestamp - lastShotTimestamp) >= irvineConfiguration.bluetooth.devices[configIndex].minInterval)
    {
        doNotify = true;
    }

    if (doNotify)
    {
        M52PASData data(device.getUnixTimestamp(), configIndex, temperature, humidity, battery, rssi);

        DataHandler::handleData(data);

        lastTemperature = temperature;
        lastHumidity = humidity;
        lastBattery = battery;
        lastShotTimestamp = currTimestamp;
    }
}
