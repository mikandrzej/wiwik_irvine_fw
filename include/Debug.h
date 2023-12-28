#pragma once

#include <Arduino.h>

enum DebugLevels {
    DEBUG_NONE,
    DEBUG_ERROR,
    DEBUG_WARNING,
    DEBUG_LOG
};

class Debug
{
public:
    void debug(const String &str);
    void warning(const String &str);
    void error(const String &str);
};

extern Debug debug;