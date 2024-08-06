#include "BluetoothJaaleeTempSensor.h"
#include <Logger.h>
#include <IrvineConfiguration.h>
#include <DataHandler.h>
#include <Device.h>

const char MODULE[] = "JAALEE";

typedef struct
{
    uint8_t dummy[25u];
    uint8_t rawTemperature[2u];
    uint8_t rawHumidity[2u];
    uint8_t dummy2[1u];
    uint8_t battery;

} __attribute__((packed)) JaaleeAdvertisedDataStruct;

BluetoothJaaleeTempSensor::BluetoothJaaleeTempSensor(const uint16_t configIndex) : AbstractBluetoothBleDevice(configIndex)
{
}

void BluetoothJaaleeTempSensor::parseAdvertisedData(const uint8_t *const data, const uint16_t len, int16_t rssi)
{
    if (len != sizeof(JaaleeAdvertisedDataStruct))
    {
        logger.logPrintF(LogSeverity::WARNING, MODULE, "Incorrect advertised data length: %d. Expected %d", len, sizeof(JaaleeAdvertisedDataStruct));
        return;
    }

    const JaaleeAdvertisedDataStruct *const structedData = (const JaaleeAdvertisedDataStruct *)data;

    uint16_t rawTemperature = (uint16_t)structedData->rawTemperature[0u] << 8 | structedData->rawTemperature[1u];
    uint16_t rawHumidity = (uint16_t)structedData->rawHumidity[0u] << 8 | structedData->rawHumidity[1u];
    float temperature = ((float)rawTemperature / 65535.0f) * 175.0f - 45.0f;
    float humidity = (float)rawHumidity / 655.35f;
    float battery = (float)structedData->battery;

    bool doNotify = false;

    uint32_t currTimestamp = millis();

    if (firstRead)
    {
        doNotify = true;
        firstRead = false;
    }
    else if ((currTimestamp - lastShotTimestamp) >= irvineConfiguration.bluetooth.devices[configIndex].maxInterval)
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

        auto data = JaaleeData(device.getUnixTimestamp(), configIndex, temperature, humidity, battery, rssi);
        DataHandler::handleJaaleeTemperatureData(data);

        lastTemperature = temperature;
        lastHumidity = humidity;
        lastBattery = battery;
        lastShotTimestamp = currTimestamp;
    }
}
