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
    int txt_len = sprintf(txt, "M53PAS (RSSI=%d) data: ", rssi);
    for (uint16_t k = 0u; k < len; k++)
    {
        txt_len += sprintf(&txt[txt_len], "%02X", data[k]);
    }

    logger.logPrintF(LogSeverity::DEBUG, MODULE, txt);
    // return;

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

    logger.logPrintF(LogSeverity::INFO, MODULE, "sensor %d temp: %.2f hum: %.2f bat: %.0f%% rssi: %d", configIndex, temperature, humidity, battery, rssi);

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
    else
    {
        if (irvineConfiguration.bluetooth.devices[configIndex].maxTemperatureChange > 0.0f)
        {
            float temperatureDiff = lastTemperature - temperature;
            if (temperatureDiff < 0.0f)
                temperatureDiff *= -1.0f;

            if (temperatureDiff > irvineConfiguration.bluetooth.devices[configIndex].maxTemperatureChange)
            {
                doNotify = true;
            }
        }
        if (irvineConfiguration.bluetooth.devices[configIndex].maxHumidityChange > 0.0f)
        {
            float humidityDiff = lastHumidity - humidity;
            if (humidityDiff < 0.0f)
                humidityDiff *= -1.0f;

            else if (humidityDiff > irvineConfiguration.bluetooth.devices[configIndex].maxHumidityChange)
            {
                doNotify = true;
            }
        }
    }

    if (doNotify)
    {
        logger.logPrintF(LogSeverity::INFO, MODULE, "sensor %d temp: %.2f hum: %.2f bat: %.0f%% rssi: %d", configIndex, temperature, humidity, battery, rssi);

        auto data = M52PASData(device.getUnixTimestamp(), configIndex, temperature, humidity, battery, rssi);
        DataHandler::handleM52PASTemperatureData(data);

        lastTemperature = temperature;
        lastHumidity = humidity;
        lastBattery = battery;
        lastShotTimestamp = currTimestamp;
    }
}