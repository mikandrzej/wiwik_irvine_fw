#include "UdsVehicleFeulLevelQuery.h"

#include "DataHandler.h"
#include <Device.h>

#include "Logger.h"
#include <IrvineConfiguration.h>

const char MODULE[] = "UDS_FUEL";

UdsVehicleFuelLevelQuery udsVehicleFuelLevelQuery;

void UdsVehicleFuelLevelQuery::loop()
{
    if (irvineConfiguration.obd.fuelLevelActive && checkTxInterval())
    {
        sendQuery();
    }
}

uint32_t UdsVehicleFuelLevelQuery::getIntervalValue()
{
    return irvineConfiguration.obd.speedInterval;
}

uint32_t UdsVehicleFuelLevelQuery::getTimeoutValue()
{
    return irvineConfiguration.obd.speedInterval * 3u;
}

bool UdsVehicleFuelLevelQuery::parseCurrDataResponse(uint8_t dataLen, uint8_t *data)
{
    if (dataLen != 1u)
        return false;
    
    uint8_t value = data[0];


    lastValue = (float)value / 2.55f;
    valueReceived = true;

    // DataHandler::handleData(*this);

    logger.logPrintF(LogSeverity::DEBUG, MODULE, "UDS FuelLevel: %f", lastValue);

    return true;
}

float UdsVehicleFuelLevelQuery::getFuelLevel(bool *valid)
{
    if (valid)
    {
        *valid = !checkRxTimeout();
    }
    return lastValue;
}

String UdsVehicleFuelLevelQuery::logData()
{
    char txt[30];
    (void)sprintf(txt, "%llu;%u\r\n",
                  device.getUnixTimestamp(),
                  lastValue);
    return String(txt);
}
String UdsVehicleFuelLevelQuery::logItem() { return "obdFuelLevel"; }

String UdsVehicleFuelLevelQuery::logMqttData()
{
    char txt[50];
    sprintf(txt, R"({"fuel":%.1f,"t":%llu})", lastValue, device.getUnixTimestamp());
    return String(txt);
}