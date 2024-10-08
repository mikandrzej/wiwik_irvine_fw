#include "Logger.h"

Logger logger;

void Logger::begin(HardwareSerial *serial)
{
    this->serial = serial;
}

void Logger::logPrintF(const LogSeverity severity, const char *module, const char *format, ...)
{
    if (severity <= currentSeverity && serial)
    {
        putHeader(severity, module);

        char buffer[256]; // Define a buffer for the formatted output, adjust size as needed
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args); // Use vsnprintf to format the string with va_list
        va_end(args);
        serial->print(buffer); // Use serial->print to output the formatted string
        serial->print("\r\n");
    }
}

void Logger::putHeader(const LogSeverity severity, const char *module)
{
    unsigned long time = millis();

    char sev[2] = {0};
    switch (severity)
    {
    case LogSeverity::DEBUG:
        sev[0] = 'D';
        break;
    case LogSeverity::INFO:
        sev[0] = 'I';
        break;
    case LogSeverity::WARNING:
        sev[0] = 'W';
        break;
    case LogSeverity::ERROR:
        sev[0] = 'E';
        break;
    default:
        break;
    }

    uint16_t ms = time % 1000;
    time /= 1000;
    uint16_t secs = time % 60;
    time /= 60;
    uint16_t mins = time % 60;
    time /= 60;
    uint16_t hours = time % 24;
    time /= 24;
    uint16_t days = time;

    serial->printf("[%s %03u:%02u:%02u:%02u:%03u %s] ", sev, days, hours, mins, secs, ms, module);
}
