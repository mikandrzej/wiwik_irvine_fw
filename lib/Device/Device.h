#pragma once

#include <stdint.h>

class Device
{
public:
    uint64_t getUnixTimestamp();

    void updateGpsTime(uint64_t unixTime);
    void updateGsmTime(uint64_t unixTime);
    void updateNtpTime(uint64_t unixTime);

private:
    uint64_t getFallbackUnixTimestamp();
    uint64_t calculateUnixTimestamp(const uint64_t lastValue, const uint32_t lastTimestamp);

    uint64_t gpsUnixTime = 0u;
    uint32_t gpsUnixTimeTimestamp = 0u;

    uint64_t gsmUnixTime = 0u;
    uint32_t gsmUnixTimeTimestamp = 0u;

    uint64_t ntpUnixTime = 0u;
    uint32_t ntpUnixTimeTimestamp = 0u;
};

extern Device device;