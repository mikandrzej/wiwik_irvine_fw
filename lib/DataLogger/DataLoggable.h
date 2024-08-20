#pragma once

#include <Arduino.h>

class DataLoggable
{
public:
    virtual String logData() { return String(); };
    virtual String logItem() { return String(); };
    virtual String logMqttData() { return String(); };
};