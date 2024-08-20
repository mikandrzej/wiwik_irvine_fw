#include "JaaleeData.h"

JaaleeData::JaaleeData()
{
}

JaaleeData::JaaleeData(
    uint64_t unixTimestamp,
    uint8_t bleConfigId,
    float temperature,
    float humidity,
    float battery,
    int16_t rssi) : bleConfigId(bleConfigId),
                    temperature(temperature),
                    humidity(humidity),
                    battery(battery),
                    rssi(rssi),
                    unixTimestamp(unixTimestamp)
{
}