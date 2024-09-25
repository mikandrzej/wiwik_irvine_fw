#pragma once

#include <vector>

class Vehicle
{
public:
    void init();
    void loop();

    float getVccVoltage(bool *valid, const bool raw = false);
    bool isIgnitionOn(bool *valid);
    bool isEngineRunningBasedOnVoltage(bool *valid);
    bool isMoving(bool *valid);
    float getSpeed(bool *valid);

private:
    void obtainVccVoltage();
    void adcCalibration();
    float calculateAverageVoltage(); 

    bool calibrationEnabled = false;
    float voltage = -1.0f;

    size_t filterDepth = 20; // Default filter depth
    std::vector<float> voltageBuffer;
    size_t currentBufferIndex = 0;
    bool bufferFilled = false;
};

extern Vehicle vehicle;