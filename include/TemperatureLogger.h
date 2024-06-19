#pragma once
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

class TemperatureLogger
{
public:
    struct Measurement
    {
        uint32_t sequence_number;
        uint64_t timestamp;
        float value;
    };

    bool init(String sensorAddress);
    bool saveData(const Measurement &data);
    // bool readDataRange(uint32_t startSeq, uint32_t endSeq, QueueHandle_t outputQueue);

private:
    String sensorAddress;
    String fileName;
    static SemaphoreHandle_t sdMutex;
    static bool initialized;
};