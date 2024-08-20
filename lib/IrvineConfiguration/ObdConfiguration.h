#pragma once

enum class ObdProtocolType
{
    NONE,
    AA_AT_THE_END
};

class ObdConfiguration
{
public:
    ObdProtocolType protocolType = ObdProtocolType::NONE;
    bool speedActive = 0u;
    uint32_t speedInterval = 0u;
    bool engineSpeedActive = 0u;
    uint32_t engineSpeedInterval = 0u;
    bool voltageActive = 0u;
    uint32_t voltageInterval = 0u;
    bool fuelLevelActive = 0u;
    uint32_t fuelLevelInterval = 0u;
    bool VINActive = 0u;
    uint32_t VINInterval = 0u;
    bool activeDTCActive = 0u;
    uint32_t activeDTCInterval = 0u;
    bool oilTemperatureActive = 0u;
    uint32_t oilTemperatureInterval = 0u;
    bool coolantTemperatureActive = 0u;
    uint32_t coolantTemperatureInterval = 0u;
    bool throttlePositionActive = 0u;
    uint32_t throttlePositionInterval = 0u;
    bool odometerActive = 0u;
    uint32_t odometerInterval = 0u;
};
