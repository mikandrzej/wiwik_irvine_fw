#pragma once

enum class TimeSource
{
    GPS,
    GSM,
    NTP
};

class TimeConfiguration
{
public:
    TimeSource source = TimeSource::GSM;
    char ntpServer[50u] = "";
    uint32_t syncInterval = 0u;
};
