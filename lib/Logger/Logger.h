#pragma once

#include <Arduino.h>

enum class LogSeverity
{
    ERROR,
    WARNING,
    INFO,
    DEBUG
};

class Logger
{
public:
    void begin(HardwareSerial *serial);
    void logPrintF(const LogSeverity severity, const char *module, const char *format, ...);

private:
    void putHeader(const LogSeverity severity, const char *module);
    LogSeverity currentSeverity = LogSeverity::ERROR;
    HardwareSerial *serial = nullptr;
};

extern Logger logger;
