#include "TemperatureLogger.h"

#define BOARD_BOARD_SD_CS_PIN (13)

bool TemperatureLogger::initialized = false;
SemaphoreHandle_t TemperatureLogger::sdMutex = xSemaphoreCreateMutex();

bool TemperatureLogger::init(String sensorAddress)
{
    if (initialized)
    {
        return true;
    }
    if (SD.begin(BOARD_BOARD_SD_CS_PIN))
    {
        Serial.println("SD initialized");

        uint8_t cardType = SD.cardType();
        if (cardType == CARD_MMC)
        {
            Serial.println("MMC");
        }
        else if (cardType == CARD_SD)
        {
            Serial.println("SDSC");
        }
        else if (cardType == CARD_SDHC)
        {
            Serial.println("SDHC");
        }
        else
        {
            Serial.println("UNKNOWN");
        }
        uint64_t cardSize = SD.cardSize() / (1024 * 1024);
        Serial.printf("SD Card Size: %lluMB\n", cardSize);

        // SD.mkdir("/temperature");

        this->sensorAddress = sensorAddress;
        this->sensorAddress.replace(":", "");
        this->fileName = String("/") + String("jaale_0x") + this->sensorAddress + String(".txt");
        initialized = true;
    }
    else
    {
        Serial.println("SD Init error");
    }
    return initialized;
}

bool TemperatureLogger::saveData(const Measurement &data)
{
    if (!initialized)
    {
        Serial.println("SD not initialized");
        return false;
    }

    if (xSemaphoreTake(this->sdMutex, portMAX_DELAY))
    {
        File file = SD.open(fileName, FILE_APPEND);
        if (!file)
        {
            Serial.println("SD write error");
            xSemaphoreGive(sdMutex);
            return false;
        }
        file.printf("%u;%lu,%.2f\n", data.sequence_number, data.timestamp, data.value);
        file.close();
        xSemaphoreGive(sdMutex);

        Serial.println("measurement saved");
        return true;
    }
    else
    {

        Serial.println("semaphore take error");
    }
    return false;
}