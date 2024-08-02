#pragma once

class VehicleConfiguration
{
public:
    uint8_t ignitionSource = 0u;          // 0: voltage, 1: CAN engine speed
    uint8_t movementDetectionSource = 0u; // 0: Accelerometer, 1: GNSS speed, 2: CAN speed
    uint8_t speedSource = 0u;             // 0: GNSS speed, 1: CAN speed
    uint32_t movementLogInterval = 0u;
    uint32_t stopLogInterval = 0u;
    uint32_t fuelLeakDetectionThreshold = 0u;
    bool speedInconsistentDetection = false;
    bool ignitionLock = false; // beacon bluetooth
    uint8_t overspeedTreshold;
    uint8_t agresiveScoreTreshold;
    // Add more parameters as needed
};
