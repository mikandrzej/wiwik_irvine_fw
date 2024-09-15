#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <GpsData.h>
#include <MqttTxItem.h>

#define MODEM_GPS_RX_QUEUE_LENGTH 1
#define MODEM_GPS_RX_ITEM_SIZE sizeof(GpsData)

#define MODEM_MQTT_TX_QUEUE_LENGTH 100
#define MODEM_MQTT_TX_ITEM_SIZE sizeof(MqttTxItem)

class Queues
{
public:
    bool begin();

    QueueHandle_t modemGpsRxQueue;
    QueueHandle_t modemMqttTxQueue;
};

extern Queues queues;