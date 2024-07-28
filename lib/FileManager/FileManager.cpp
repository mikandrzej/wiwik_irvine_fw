#include "FileManager.h"

#include <SPI.h>

bool FileManager::init()
{
    SPI.begin(SD_SCLK_PIN, SD_MISO_PIN, SD_MOSI_PIN);
    if (!SD.begin(SD_CS_PIN, SPI))
        return false;

    cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card attached");
        return false;
    }

    Serial.print("SD Card Type: ");
    switch (cardType)
    {
    case CARD_MMC:
        Serial.println("MMC");
        break;
    case CARD_SD:
        Serial.println("SDSC");
        break;
    case CARD_SDHC:
        Serial.println("SDHC");
        break;
    default:
        Serial.println("UNKNOWN");
        break;
    }

    cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

    return true;
}

bool FileManager::createDirectory(String &path)
{
    return SD.mkdir(path);
}

File FileManager::createFile(String &path, const char *mode)
{
    return SD.open(path, mode, true);
}
