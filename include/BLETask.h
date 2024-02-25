#pragma once

#include <stdint.h>
#include <freertos/queue.h>

typedef enum
{
    BLE_QUE_TYPE_TEMPERATURE
} ble_que_type_e;

typedef struct
{
    ble_que_type_e type;
    union
    {
        struct
        {
            char sensor_address[30];
            float value;
        } temperature;
    };
} ble_que_item_s;

#define BLE_QUEUE_LENGTH 1
#define BLE_QUEUE_ITEM_SIZE sizeof(ble_que_item_s)

extern QueueHandle_t xBleQueue;

extern void BLETask(void *pvParameters);