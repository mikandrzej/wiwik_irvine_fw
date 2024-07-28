#include "DataLogger.h"
#include <TimeLib.h>

#include <FS.h>
#include <SD.h>

DataLogger::DataLogger(String &&sensorName, String &&path) : sensorName(sensorName), path(path)
{
}

bool DataLogger::init()
{
    String fullPath = path + sensorName + ".csv";
    file = SD.open(fullPath, "w", true);
    return file.available();
}

bool DataLogger::logData(const double value)
{
    String line = String(now()) + ";" + String(value, 4);
    return saveData(line);
}

bool DataLogger::logData(const int value)
{
    String line = String(now()) + ";" + String(value);
    return saveData(line);
}

bool DataLogger::saveData(String &data)
{
    data += "\r\n";

    size_t savedLen = file.write((const uint8_t *)data.c_str(), data.length());
    linesToSave++;

    if (linesToSave > 10)
    {
        file.flush();
        linesToSave = 0;
    }

    return savedLen == data.length();
}
