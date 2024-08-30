#include "CanQuery.h"

#include <Logger.h>
#include "../CanTxQueue.h"

const char MODULE[] = "CAN_QUERY";

bool CanQuery::checkTxInterval()
{
    bool shot = false;

    if ((millis() - lastShotTimestamp) > getIntervalValue())
        shot = true;
    shot |= first;

    return shot;
}

bool CanQuery::checkRxTimeout()
{
    if (!waitingForResponse)
        return false;
    if ((millis() - lastResponseTimestamp) > getTimeoutValue())
        return true;
    return false;
}

void CanQuery::sendFrame(twai_message_t &msg)
{
    lastShotTimestamp = millis();
    first = false;
    waitingForResponse = true;

    // logger.logPrintF(LogSeverity::DEBUG, MODULE, "Send frame id:%X data(%u):%02X%02X%02X%02X%02X%02X%02X%02X",
    //                  msg.identifier,
    //                  msg.data_length_code,
    //                  msg.data[0],
    //                  msg.data[1],
    //                  msg.data[2],
    //                  msg.data[3],
    //                  msg.data[4],
    //                  msg.data[5],
    //                  msg.data[6],
    //                  msg.data[7]);
    canTxQueue.push(msg);
}

bool CanQuery::frameReceived(twai_message_t &msg)
{
    if (parseReceivedFrame(msg))
    {
        lastResponseTimestamp = millis();

        waitingForResponse = false;
        return true;
    }
    return false;
}
