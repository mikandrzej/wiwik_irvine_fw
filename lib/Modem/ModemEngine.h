#pragma once
#include <Arduino.h>

#include <AtCommand.hpp>
#include <vector>

class ModemEngine
{

public:
    ModemEngine(HardwareSerial &serial);
    void loop();
    void registerCommand(AtCommand &command);

private:
    std::array<char, 4096> cmdRxBuf;
    uint32_t cmdRxBufPos = 0u;
    std::vector<AtCommand *> registeredCommands;
    HardwareSerial &serial;

    void parseReceivedLine();
};