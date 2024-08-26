#pragma once

#include <driver/twai.h>
#include <queue>

extern std::queue<twai_message_t> canTxQueue;