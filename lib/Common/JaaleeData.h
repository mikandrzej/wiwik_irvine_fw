#pragma once

#include <stdint.h>
#include <DataLoggable.h>
#include <IrvineConfiguration.h>

class JaaleeData : public DataLoggable
{
public:
    JaaleeData();
    JaaleeData(
        uint64_t unixTimestamp,
        uint8_t bleConfigId,
        float temperature,
        float humidity,
        float battery,
        int16_t rssi);
    uint8_t bleConfigId = 0u;
    float temperature = 0.0f;
    float humidity = 0.0f;
    float battery = 0.0f;
    int16_t rssi = 0;
    uint64_t unixTimestamp = 0u;

    String logData()
    {
        char txt[200];
        (void)sprintf(txt, "%llu;%f;%f;%f;%d\r\n",
                      unixTimestamp,
                      temperature,
                      humidity,
                      battery,
                      rssi);
        return String(txt);
    }
    String logItem()
    {
        char txt[20];
        sprintf(txt, "jaalee_%02x%02x%02x%02x%02x%02x",
                irvineConfiguration.bluetooth.devices[bleConfigId].macAddress[0],
                irvineConfiguration.bluetooth.devices[bleConfigId].macAddress[1],
                irvineConfiguration.bluetooth.devices[bleConfigId].macAddress[2],
                irvineConfiguration.bluetooth.devices[bleConfigId].macAddress[3],
                irvineConfiguration.bluetooth.devices[bleConfigId].macAddress[4],
                irvineConfiguration.bluetooth.devices[bleConfigId].macAddress[5]);
        return String(txt);
    }
    String logMqttData()
    {
        char msg[100];
        sprintf(msg, R"({"temp":%.2f,"hum":%.2f,"bat":%.0f,"rssi":%d,"t":%llu})", temperature, humidity, battery, rssi, unixTimestamp);
        return String(msg);
    }
};