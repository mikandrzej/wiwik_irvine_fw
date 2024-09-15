#include "MqttPublisher.h"

#include <ModemManagement.h>
#include <Device.h>
#include <IrvineConfiguration.h>

void MqttPublisher::publishSdData(uint64_t used_mb, uint64_t total_mb, uint64_t card_size)
{
    MqttTxItem txItem;
    sprintf(txItem.topic, "irvine/%s/sd", irvineConfiguration.device.deviceId);
    sprintf(txItem.msg, R"({"tim":%llu,"siz":%lu,"tot":%lu,"use":%lu})",
            device.getUnixTimestamp(),
            card_size / 1024ull,
            total_mb / 1024ull,
            used_mb / 1024ull);
    txItem.retain = false;

    modemManagement.mqttPublish(txItem);
}