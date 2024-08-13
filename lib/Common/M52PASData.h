#pragma once

#include <stdint.h>
#include <DataLoggable.h>
#include <IrvineConfiguration.h>

class M52PASData : public DataLoggable
{
public:
    M52PASData();
    M52PASData(
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
    String logData()
    {
        char txt[200];
        (void)sprintf(txt, "%llu;%f;%f;%f;%d\r\n",
                      DataLoggable::unixTimestamp,
                      temperature,
                      humidity,
                      battery,
                      rssi);
        return String(txt);
    }
    String logItem()
    {
        char txt[20];
        sprintf(txt, "m52pas_%02x%02x%02x%02x%02x%02x",
                irvineConfiguration.bluetooth.devices[bleConfigId].macAddress[0],
                irvineConfiguration.bluetooth.devices[bleConfigId].macAddress[1],
                irvineConfiguration.bluetooth.devices[bleConfigId].macAddress[2],
                irvineConfiguration.bluetooth.devices[bleConfigId].macAddress[3],
                irvineConfiguration.bluetooth.devices[bleConfigId].macAddress[4],
                irvineConfiguration.bluetooth.devices[bleConfigId].macAddress[5]);
        return String(txt);
    }
};