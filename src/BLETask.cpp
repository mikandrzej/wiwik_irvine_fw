#include <freertos/FreeRTOS.h>

#include "BLETask.h"
#include "BLEJaaleTemperature.h"
#include "Configuration.h"
#include <TemperatureLogger.h>

QueueHandle_t xBleQueue;

BleJaaleeTemperature bleTemperature;

void BLETask(void *pvParameters)
{
    Serial.println("BLE task created");
    TemperatureLogger tempLogger;

    tempLogger.init(configuration.getJaaleSensorAddress());

    bleTemperature.setAddress(configuration.getJaaleSensorAddress());
    bleTemperature.setTemperatureCallback(
        [&tempLogger](String &sensorAddress, float temperature)
        {
            tempLogger.saveData({.sequence_number = 1u, .timestamp = 2, .value = temperature});
            ble_que_item_s msg = {.temperature = {.value = temperature}};
            strncpy(msg.temperature.sensor_address, sensorAddress.c_str(), sizeof(msg.temperature.sensor_address));
            xQueueOverwrite(xBleQueue, &msg);
        });

    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        bleTemperature.loop();
        xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));
    }
}