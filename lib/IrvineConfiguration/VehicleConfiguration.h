#pragma once

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
    uint8_t ignitionSource = 0u; // 0: voltage, 1: CAN engine speed
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
