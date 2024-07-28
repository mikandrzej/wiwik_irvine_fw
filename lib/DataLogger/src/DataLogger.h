#pragma once

#include <Arduino.h>
#include <FS.h>

class DataLogger
{
public:
    DataLogger(String &&sensorName, String &&path = "/dane/");

public:
    bool init();
    bool logData(const double value);
    bool logData(const int value);

private:
    bool saveData(String &data);

private:
    String sensorName = "";
    String path = "";

    fs::File file;

    int linesToSave = {0};
};