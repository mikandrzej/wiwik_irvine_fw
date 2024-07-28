#pragma once
#include <Arduino.h>

#include <FS.h>
#include <SD.h>

#define SD_MISO_PIN 2
#define SD_MOSI_PIN 15
#define SD_SCLK_PIN 14
#define SD_CS_PIN 13

class FileManager
{
public:
    bool init();
    bool createDirectory(String &path);
    File createFile(String &path, const char *mode = "rw");

private:
    uint8_t cardType;
    uint64_t cardSize;
};
