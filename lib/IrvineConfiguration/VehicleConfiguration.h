#pragma once

enum class VehicleIgnitionSource
{
    VOLTAGE,
    CAN
};

enum class VehicleMovementDetectionSource
{
    ACCELEROMETER,
    GPS,
    CAN
};

enum class VehicleSpeedSource
{
    GPS,
    CAN
};

class VehicleConfiguration
{
public:
    VehicleIgnitionSource ignitionSource = VehicleIgnitionSource::CAN;
    VehicleMovementDetectionSource movementDetectionSource = VehicleMovementDetectionSource::GPS;
    VehicleSpeedSource speedSource = VehicleSpeedSource::CAN;
    uint32_t movementLogInterval = 0u;
    uint32_t stopLogInterval = 0u;
    uint32_t fuelLeakDetectionThreshold = 0u;
    bool speedInconsistentDetection = false;
    bool ignitionLock = false; // beacon bluetooth
    uint8_t overspeedTreshold;
    uint8_t agresiveScoreTreshold;
    // Add more parameters as needed
};
