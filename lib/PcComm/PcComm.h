#pragma once

#include <vector>
#include <Arduino.h>
#include <PcCommand.h>

class PcComm
{
public:
    PcComm(HardwareSerial &hwSerial);
    void loop();

private:
    HardwareSerial &serial;
    std::vector<PcCommand *> commands;
    char rxBuffer[1024];
    uint16_t rxBufferPos = 0u;

    void parseBuffer();
    void handleApnCommand(char *data);
    void handleSimPinCommand(char *data);
    void handleMqttServerCommand(char *data);
    void handleBleDeviceCommand(char *data);
};

extern PcComm pcComm;