#include "Device.h"
#include <IrvineConfiguration.h>

Device device;

const char MODULE[] = "DEVICE";

uint64_t Device::getUnixTimestamp()
{
    switch (irvineConfiguration.time.source)
    {
    case TimeSource::GPS:
        if (gpsUnixTime > 0)
        {
            return calculateUnixTimestamp(gpsUnixTime, gpsUnixTimeTimestamp);
        }
    case TimeSource::GSM:
        if (gsmUnixTime > 0)
        {
            return calculateUnixTimestamp(gsmUnixTime, gsmUnixTimeTimestamp);
        }
    case TimeSource::NTP:
        if (ntpUnixTime > 0)
        {
            return calculateUnixTimestamp(ntpUnixTime, ntpUnixTimeTimestamp);
        }
    }

    return getFallbackUnixTimestamp();
}

uint64_t Device::getFallbackUnixTimestamp()
{
    if (gpsUnixTime > 0)
    {
        return calculateUnixTimestamp(gpsUnixTime, gpsUnixTimeTimestamp);
    }
    else if (gsmUnixTime > 0)
    {
        return calculateUnixTimestamp(gsmUnixTime, gsmUnixTimeTimestamp);
    }
    else if (ntpUnixTime > 0)
    {
        return calculateUnixTimestamp(ntpUnixTime, ntpUnixTimeTimestamp);
    }

    return 0;
}

uint64_t Device::calculateUnixTimestamp(const uint64_t lastValue, const uint32_t lastTimestamp)
{
    logger.logPrintF(LogSeverity::DEBUG, MODULE, "lastVal: %llu, lastTimestamp: %lu", lastValue, lastTimestamp);
    uint32_t msSinceLastValue = millis() - lastTimestamp;
    msSinceLastValue /= 1000u;
    return lastValue + (uint64_t)msSinceLastValue;
}

void Device::updateGpsTime(uint64_t unixTime)
{
    logger.logPrintF(LogSeverity::DEBUG, MODULE, "Update GPS time: %llu", unixTime);
    gpsUnixTime = unixTime;
    gpsUnixTimeTimestamp = millis();
}

void Device::updateGsmTime(uint64_t unixTime)
{
    logger.logPrintF(LogSeverity::DEBUG, MODULE, "Update GSM time: %llu", unixTime);
    gsmUnixTime = unixTime;
    gsmUnixTimeTimestamp = millis();
}

void Device::updateNtpTime(uint64_t unixTime)
{
    logger.logPrintF(LogSeverity::DEBUG, MODULE, "Update NTP time: %llu", unixTime);
    ntpUnixTime - unixTime;
    ntpUnixTimeTimestamp = millis();
}
