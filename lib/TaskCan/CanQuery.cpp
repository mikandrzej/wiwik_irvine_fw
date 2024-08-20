#include "CanQuery.h"

#include <Logger.h>

const char MODULE[] = "CAN_QUERY";

void CanQuery::process(const uint32_t time)
{
    bool shot = false;

    if ((time - lastShotTimestamp) > interval)
        shot = true;
    if (first)
    {
        first = false;
        shot = true;
    }

    if (shot)
    {
        lastShotTimestamp = time;

        intervalElapsed();
    }
}

void CanQuery::intervalElapsed()
{
    logger.logPrintF(LogSeverity::DEBUG, MODULE, "Interval elapsed");
}

void CanQuery::sendFrame(twai_message_t &msg)
{
    logger.logPrintF(LogSeverity::DEBUG, MODULE, "Send frame id:%X data(%u):%02X%02X%02X%02X%02X%02X%02X%02X",
                     msg.identifier,
                     msg.data_length_code,
                     msg.data[0],
                     msg.data[1],
                     msg.data[2],
                     msg.data[3],
                     msg.data[4],
                     msg.data[5],
                     msg.data[6],
                     msg.data[7]);
    if (sendCallback)
        sendCallback(msg);
}

void CanQuery::setSendCallback(SendCallback sendCallback)
{
    this->sendCallback = sendCallback;
}
