#pragma once

#include <Arduino.h>

class DataLoggable
{
public:
    virtual String logData() = 0;
    virtual String logItem() = 0;

    uint64_t unixTimestamp = 0u;
};