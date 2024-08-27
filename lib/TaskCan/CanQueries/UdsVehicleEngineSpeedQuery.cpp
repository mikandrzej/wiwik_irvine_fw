#include "UdsVehicleEngineSpeedQuery.h"

#include "DataHandler.h"
#include <Device.h>

#include "Logger.h"
#include <IrvineConfiguration.h>

const char MODULE[] = "UDS_ENGINE";

UdsVehicleEngineSpeedQuery udsVehicleEngineSpeedQuery;

void UdsVehicleEngineSpeedQuery::loop()
{
    if (checkTxInterval())
    {
        sendQuery();
    }
}

uint32_t UdsVehicleEngineSpeedQuery::getIntervalValue()
{
    return irvineConfiguration.obd.speedInterval;
}

uint32_t UdsVehicleEngineSpeedQuery::getTimeoutValue()
{
    return irvineConfiguration.obd.speedInterval / 2u;
}

bool UdsVehicleEngineSpeedQuery::parseCurrDataResponse(uint8_t dataLen, uint8_t *data)
{
    if (dataLen != 2u)
        return false;

    lastValue = ((uint16_t)data[0u] << 8) | data[1];
    valueReceived = true;

    DataHandler::handleData(*this);

    logger.logPrintF(LogSeverity::DEBUG, MODULE, "UDS Engine speed: %u", lastValue);

    return true;
}

uint16_t UdsVehicleEngineSpeedQuery::getEngineSpeed(bool *valid)
{
    if (valid)
    {
        if (checkRxTimeout())
            *valid = false;
    }
    return lastValue;
}

String UdsVehicleEngineSpeedQuery::logData()
{
    char txt[30];
    (void)sprintf(txt, "%llu;%u\r\n",
                  device.getUnixTimestamp(),
                  lastValue);
    return String(txt);
}
String UdsVehicleEngineSpeedQuery::logItem() { return "obdEngineSpeed"; }

String UdsVehicleEngineSpeedQuery::logMqttData()
{
    char txt[50];
    sprintf(txt, R"({"eng":%u,"t":%llu})", lastValue, device.getUnixTimestamp());
    return String(txt);
}