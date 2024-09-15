#include "Queues.h"

Queues queues;

bool Queues::begin()
{
    modemGpsRxQueue = xQueueCreate(MODEM_GPS_RX_QUEUE_LENGTH, MODEM_GPS_RX_ITEM_SIZE);
    if (!modemGpsRxQueue)
        return false;

    modemMqttTxQueue = xQueueCreate(MODEM_MQTT_TX_QUEUE_LENGTH, MODEM_MQTT_TX_ITEM_SIZE);
    if (!modemMqttTxQueue)
        return false;

    return true;
}