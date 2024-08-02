#include "GpsConfiguration.h"

bool GpsConfiguration::setGpsMaxInterval(const uint32_t value)
{
    maxInterval = value;
    return true;
}