#include "GpsData.h"

GpsData::GpsData() {};

GpsData::GpsData(uint8_t mode,
                 uint8_t satellites,
                 double latitude,
                 double longitude,
                 double altitude,
                 double speed,
                 uint64_t gpsUnixTimestamp,
                 uint64_t unixTimestamp) : mode(mode),
                                           satellites(satellites),
                                           latitude(latitude),
                                           longitude(longitude),
                                           altitude(altitude),
                                           speed(speed),
                                           gpsUnixTimestamp(gpsUnixTimestamp)

{
    DataLoggable::unixTimestamp = unixTimestamp;
}