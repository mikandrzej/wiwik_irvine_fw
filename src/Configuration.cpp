#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include "File.h"
#include "Configuration.h"

#define BOARD_MISO_PIN (2)
#define BOARD_MOSI_PIN (15)
#define BOARD_SCK_PIN (14)
#define BOARD_SD_CS_PIN (13)

void Configuration::initSource()
{
    SPI.begin(BOARD_SCK_PIN, BOARD_MISO_PIN, BOARD_MOSI_PIN);
    if (!SD.begin(BOARD_SD_CS_PIN))
    {
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
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

    listDir(SD, "/", 0);

    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));

    m_sourceInit = true;
}

void Configuration::readConfig()
{
    if(!m_sourceInit)
    {
        Serial.println("SD Source not initialised. Please check your SD card.");
        return;
    }
    readReportInterval();
}

bool Configuration::readReportInterval()
{
    File file = SD.open(m_report_interval_path.c_str());
    uint8_t data[10];
    size_t result = file.read(data, sizeof(data));
    if (-1 == result)
    {
        Serial.println("Unable to read ReportInterval");
        return false;
    }

    String readData(data, result);
    m_reportInterval = readData.toInt();

    Serial.printf("Read ReportInterval value = %d\r\n", m_reportInterval);
    return true;

}

Configuration configuration;