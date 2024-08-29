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
    bool isMoving(bool *valid);
    GpsData getGpsData(bool *valid);

private:
    void parseGpsData(const String &data);
    void handleGpsData(GpsData &gpsData);
    void publishNewData(GpsData &gpsData);
    String getNextSubstring(const String &input, char separator, int *iterator);

    float degreesToRadians(float degree);

    float getDistanceFromLastShot(const GpsData &gpsData);

    GpsState state = GpsState::UNINITIALIZED;
    bool firstShot = true;
    GpsData lastReceivedData;
    bool lastShotValid = false;
    uint32_t lastShotTimestamp = 0u;

    bool moving = false;
};

extern GpsController gpsController;