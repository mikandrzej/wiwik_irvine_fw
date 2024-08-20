#include "M52PASData.h"

M52PASData::M52PASData()
{
}

M52PASData::M52PASData(
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