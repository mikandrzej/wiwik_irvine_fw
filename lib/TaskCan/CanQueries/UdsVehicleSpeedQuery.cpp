#include "UdsVehicleSpeedQuery.h"

#include "DataHandler.h"
#include <Device.h>

#include "Logger.h"
#include <IrvineConfiguration.h>

const char MODULE[] = "UDS_SPEED";

UdsVehicleSpeedQuery udsVehicleSpeedQuery;

void UdsVehicleSpeedQuery::loop()
{
    if (irvineConfiguration.obd.speedActive && checkTxInterval())
    {
        sendQuery();
    }
}

uint32_t UdsVehicleSpeedQuery::getIntervalValue()
{
    return irvineConfiguration.obd.speedInterval;
}

uint32_t UdsVehicleSpeedQuery::getTimeoutValue()
{
    return irvineConfiguration.obd.speedInterval * 3u;
}

bool UdsVehicleSpeedQuery::parseCurrDataResponse(uint8_t dataLen, uint8_t *data)
{
    if (dataLen != 1u)
        return false;

    lastValue = data[0u];
    valueReceived = true;

    logger.logPrintF(LogSeverity::DEBUG, MODULE, "UDS Speed: %u", lastValue);

    return true;
}

uint8_t UdsVehicleSpeedQuery::getSpeed(bool *valid)
{
    if (valid)
    {
        *valid = !checkRxTimeout();
    }
    return lastValue;
}

String UdsVehicleSpeedQuery::logData()
{
    char txt[30];
    (void)sprintf(txt, "%llu;%u\r\n",
                  device.getUnixTimestamp(),
                  lastValue);
    return String(txt);
}
String UdsVehicleSpeedQuery::logItem() { return "obdSpeed"; }

String UdsVehicleSpeedQuery::logMqttData()
{
    char txt[50];
    sprintf(txt, R"({"spd":%u,"t":%llu})", lastValue, device.getUnixTimestamp());
    return String(txt);
}