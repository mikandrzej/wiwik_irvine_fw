#include <freertos/FreeRTOS.h>

#include "BLETask.h"
#include "BLEJaaleTemperature.h"
#include "Configuration.h"

QueueHandle_t xBleQueue;

BleJaaleeTemperature bleTemperature;

void BLETask(void *pvParameters)
{
    Serial.println("BLE task created");

    bleTemperature.setAddress(configuration.getJaaleSensorAddress());
    bleTemperature.setTemperatureCallback(
        [](String &sensorAddress, float temperature)
        {
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