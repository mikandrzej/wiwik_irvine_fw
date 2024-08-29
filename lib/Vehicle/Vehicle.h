#pragma once


class Vehicle
{
public:
    void init();
    void loop();


    float getVccVoltage(bool *valid);
    bool isIgnitionOn(bool *valid);
    bool isEngineRunningBasedOnVoltage(bool *valid);
    bool isMoving(bool *valid);
    float getSpeed(bool *valid);

private:
    void obtainVccVoltage();

    void adcCalibration();


    bool calibrationEnabled = false;
    float voltage = -1.0f;
};

extern Vehicle vehicle;