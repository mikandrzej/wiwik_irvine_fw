#pragma once

#include <functional>
#include <stdint.h>
#include <driver/twai.h>
#include <vector>

class CanQuery
{
public:
    virtual void loop() = 0;

    bool frameReceived(twai_message_t &msg);

protected:
    bool checkTxInterval();
    bool checkRxTimeout();
    void sendFrame(twai_message_t &msg);

    virtual bool parseReceivedFrame(twai_message_t &msg) = 0;
    virtual uint32_t getIntervalValue() = 0;
    virtual uint32_t getTimeoutValue() = 0;

    uint32_t lastShotTimestamp = 0u;
    uint32_t lastResponseTimestamp = 0u;

    bool waitingForResponse = false;
    bool first = true;
};