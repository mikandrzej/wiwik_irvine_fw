#pragma once

#include <GpsData.h>

class GpsObserver
{
public:
    bool begin();
    void loop();

private:
    void handleNewGpsData(GpsData &gpsData);
    float getDistanceFromLastShot(const GpsData &gpsData);

    float degreesToRadians(float degree);

    GpsData lastReceivedData;
    bool firstMeasure = true;
    bool moving = false;
    uint32_t lastShotTimestamp = 0u;
};

extern GpsObserver gpsObserver;