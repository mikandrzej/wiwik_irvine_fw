#include "UdsVehicleSpeedQuery.h"

#include "DataHandler.h"
#include <Device.h>

#include "Logger.h"

const char MODULE[] = "UDS_SPEED";

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

UdsVehicleSpeedQuery::UdsVehicleSpeedQuery(uint32_t interval) : UdsCurrDataQuery(0x0Du)
{
    this->interval = interval;
}

bool UdsVehicleSpeedQuery::parseReceivedValue(uint8_t dataLen, uint8_t *data)
{
    if (dataLen != 1u)
        return false;

    lastValue = data[0u];
    valueReceived = true;

    DataHandler::handleData(*this);

    logger.logPrintF(LogSeverity::DEBUG, MODULE, "UDS Speed: %u", lastValue);

    return true;
}

uint8_t UdsVehicleSpeedQuery::getLastValue()
{
    return lastValue;
}
