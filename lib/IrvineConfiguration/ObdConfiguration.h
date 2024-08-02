#pragma once

class ObdConfiguration
{
public:
    uint8_t protocol = 0;
    bool speed = false;
    bool engineSpeed = false;
    bool voltage = false;
    bool fuelLevel = false;
    char VIN[20u] = "";
    bool activeDTC = false;
    bool oilTemperature = false;
    bool coolantTemperature = false;
    bool throttlePosition = false;
    bool odometer = false;
};
