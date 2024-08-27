#pragma once

#include <vector>

#include "CanQueries/CanQuery.h"
#include <driver/twai.h>

class CanManager
{
public:
    void setup();
    void loop();

private:
    void sendCallback(twai_message_t &msg);

    std::vector<CanQuery *> canQueries;

    uint32_t lastSentTimestamp = 0u;
    uint32_t sendInterval = 50u;
};

extern CanManager canManager;