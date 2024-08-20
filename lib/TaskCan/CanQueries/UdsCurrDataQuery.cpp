#include "UdsCurrDataQuery.h"

#include "Logger.h"

const char MODULE[] = "UDS_CURR";

UdsCurrDataQuery::UdsCurrDataQuery(uint8_t pid)
    : CanQuery(), pid(pid)
{
    switch (irvineConfiguration.obd.protocolType)
    {
    case ObdProtocolType::AA_AT_THE_END:
        queryId = 0x7DFu;
        extd = false;
        padding = true;
        paddingChar = 0xAAu;
        responseId = 0x7E8u;

        logger.logPrintF(LogSeverity::DEBUG, MODULE, "Added UDS Current Data PID:%02X", pid);
        break;
    }
}

bool UdsCurrDataQuery::messageReceived(uint32_t id, bool extd, bool rtr, uint8_t dataLen, uint8_t *data)
{
    if (id != responseId)
    {
        logger.logPrintF(LogSeverity::WARNING, MODULE, "Invalid id %X", id);
        return false;
    }
    if (extd != this->extd)
    {
        logger.logPrintF(LogSeverity::WARNING, MODULE, "Extd doesn't match %d", extd);
        return false;
    }
    if (rtr != this->rtr)
    {
        logger.logPrintF(LogSeverity::WARNING, MODULE, "RTR doesn't match %d", rtr);
        return false;
    }

    UdsFrame *frame = (UdsFrame *)data;
    if (padding)
    {
        if (dataLen != 8u)
        {
            logger.logPrintF(LogSeverity::WARNING, MODULE, "Invalid data length %d", dataLen);
            return false;
        }
    }
    else if (dataLen >= 3u)
    {
        if (frame->additionalBytes + 1u != dataLen)
        {
            logger.logPrintF(LogSeverity::WARNING, MODULE, "Invalid data length %d with additional bytes %d", dataLen, frame->additionalBytes);
            return false;
        }
    }
    if (frame->service != (UDS_SERVICE_CURR_DATA | 0x40u))
    {
        logger.logPrintF(LogSeverity::WARNING, MODULE, "Invalid service", frame->service);
        return false;
    }
    if (frame->pid != pid)
    {
        logger.logPrintF(LogSeverity::WARNING, MODULE, "Invalid PID", frame->pid);
        return false;
    }

    logger.logPrintF(LogSeverity::DEBUG, MODULE, "Received correct message PID:%X", pid);

    return parseReceivedValue(frame->additionalBytes - 2u, frame->data);
}

void UdsCurrDataQuery::sendQuery()
{
    twai_message_t msg{0};

    switch (irvineConfiguration.obd.protocolType)
    {
    case ObdProtocolType::AA_AT_THE_END:
        msg.identifier = queryId;
        msg.extd = false;
        msg.rtr = false;
        msg.data_length_code = 8u;
        UdsFrame *udsPart = (UdsFrame *)msg.data;
        udsPart->service = UDS_SERVICE_CURR_DATA;
        udsPart->pid = pid;
        udsPart->additionalBytes = 2u;
        memset(udsPart->data, 0xAA, sizeof(udsPart->data));

        sendFrame(msg);

        logger.logPrintF(LogSeverity::DEBUG, MODULE, "send Query PID:%X", pid);
        break;
    }
}

void UdsCurrDataQuery::intervalElapsed()
{
    logger.logPrintF(LogSeverity::DEBUG, MODULE, "Interval elapsed");
    sendQuery();
}
