#include "Device.h"
#include <IrvineConfiguration.h>
#include <Logger.h>

const char MODULE[] = "DEVICE";

Device device;

uint64_t Device::getUnixTimestamp()
{
    switch (irvineConfiguration.time.source)
    {
    case TimeSource::GPS:
        if (gpsUnixTime > 0)
        {
            return calculateUnixTimestamp(gpsUnixTime, gpsUnixTimeTimestamp);
        }
        break;
    case TimeSource::GSM:
        if (gsmUnixTime > 0)
        {
            return calculateUnixTimestamp(gsmUnixTime, gsmUnixTimeTimestamp);
        }
        break;
    case TimeSource::NTP:
        if (ntpUnixTime > 0)
        {
            return calculateUnixTimestamp(ntpUnixTime, ntpUnixTimeTimestamp);
        }
        break;
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
    uint32_t msSinceLastValue = millis() - lastTimestamp;
    msSinceLastValue /= 1000u;
    // logger.logPrintF(LogSeverity::DEBUG, MODULE, "msSince: %lu, lastVal: %llu, lastTimestamp: %lu", msSinceLastValue, lastValue, lastTimestamp);
    return lastValue + (uint64_t)msSinceLastValue;
}

void Device::updateGpsTime(uint64_t unixTime)
{
    gpsUnixTime = unixTime;
    gpsUnixTimeTimestamp = millis();
}

void Device::updateGsmTime(uint64_t unixTime)
{
    gsmUnixTime = unixTime;
    gsmUnixTimeTimestamp = millis();
}

void Device::updateNtpTime(uint64_t unixTime)
{
    ntpUnixTime - unixTime;
    ntpUnixTimeTimestamp = millis();
}
