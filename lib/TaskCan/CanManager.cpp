#include "CanManager.h"
#include "IrvineConfiguration.h"

#include <Logger.h>
#include <DataLogger.h>

#include <driver/twai.h>
#include "CanTxQueue.h"

const char MODULE[] = "CAN_MGR";

CanManager canManager;

void CanManager::setup()
{
    // if (irvineConfiguration.obd.speedActive)
    // {
    //     logger.logPrintF(LogSeverity::DEBUG, MODULE, "Added UDS Vehicle speed query");
    //     auto query = new UdsVehicleSpeedQuery(irvineConfiguration.obd.speedInterval);
    //     query->setSendCallback([this](twai_message_t &msg)
    //                            { this->sendCallback(msg); });
    //     canQueries.push_back(query);
    // }
}

void CanManager::loop()
{
    uint32_t time = millis();
    twai_message_t rcvdMessage;
    bool messageRcvd = twai_receive(&rcvdMessage, 0u) == ESP_OK;

    if(messageRcvd)
    {
        udsVehicleSpeedQuery.frameReceived(rcvdMessage);
    }

    udsVehicleSpeedQuery.loop();

    for (auto &query : canQueries)
    {
        if (messageRcvd)
        {
            query->frameReceived(rcvdMessage);
        }
        query->loop();
    }


    if ((time - lastSentTimestamp) >= sendInterval)
    {
        if (canTxQueue.size() > 0)
        {

            twai_message_t msgToSend = canTxQueue.front();
            canTxQueue.pop();

            logger.logPrintF(LogSeverity::DEBUG, MODULE, "sending frame %X: (%u)%X%X%X%X%X%X%X%X. extd:%d",
                             msgToSend.identifier,
                             msgToSend.data_length_code,
                             msgToSend.data[0u],
                             msgToSend.data[1u],
                             msgToSend.data[2u],
                             msgToSend.data[3u],
                             msgToSend.data[4u],
                             msgToSend.data[5u],
                             msgToSend.data[6u],
                             msgToSend.data[7u],
                             msgToSend.extd);

            esp_err_t status = twai_transmit(&msgToSend, 100u);

            logger.logPrintF(LogSeverity::DEBUG, MODULE, "twai transmit status: %x", status);

            lastSentTimestamp = time;
        }
    }
}

void CanManager::sendCallback(twai_message_t &msg)
{
    logger.logPrintF(LogSeverity::DEBUG, MODULE, "sendCallback called");
    canTxQueue.push(msg);
}

