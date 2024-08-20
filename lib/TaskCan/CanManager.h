#pragma once

#include <vector>
#include <queue>

#include "CanQuery.h"
#include <driver/twai.h>

class CanManager
{
public:
    void setup();
    void loop();

    void sendCallback(twai_message_t &msg);

private:
    std::vector<CanQuery *> canQueries;

    std::queue<twai_message_t> framesToSend;

    uint32_t lastSentTimestamp = 0u;
    uint32_t sendInterval = 50u;
};

extern CanManager canManager;