#pragma once

#include <Arduino.h>
#include <GpsData.h>
#include <memory>

enum class GpsState
{
    UNINITIALIZED,
    READY
};

class GpsController
{
public:
    void loop();
    bool isMoving();

private:
    void parseGpsData(const String &data);
    void handleGpsData(const GpsData &gpsData);
    void publishNewData(const GpsData &gpsData);
    String getNextSubstring(const String &input, char separator, int *iterator);

    float degreesToRadians(float degree);

    float getDistanceFromLastShot(const GpsData &gpsData);

    GpsState state = GpsState::UNINITIALIZED;
    bool firstShot = true;
    GpsData lastPublishedData;
    uint32_t lastShotTimestamp = 0u;

    bool moving = false;
};

extern GpsController gpsController;