#pragma once

#include <functional>
#include <stdint.h>
#include <driver/twai.h>
#include <vector>

using SendCallback = std::function<void(twai_message_t &)>;

class CanQuery
{
public:
    CanQuery() {};
    CanQuery(uint32_t interval, uint32_t responseTimeout, bool extd = false, bool rtr = false)
        : interval(interval),
          responseTimeout(responseTimeout),
          extd(extd),
          rtr(rtr)
    {
    }

    void process(const uint32_t time);

    virtual void intervalElapsed();

    virtual bool messageReceived(uint32_t id, bool extd, bool rtr, uint8_t data_len, uint8_t *data) = 0;

    void sendFrame(twai_message_t &msg);

    void setSendCallback(SendCallback sendCallback);

protected:
    uint32_t interval = 0u;
    uint32_t responseTimeout = 0u;
    uint32_t lastShotTimestamp = 0u;
    bool extd = false;
    bool rtr = false;

    bool first = true;

    SendCallback sendCallback = nullptr;
};