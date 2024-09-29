#pragma once

#include <vector>
#include <Arduino.h>
#include <PcCommand.h>
#include <SimpleIntervalTimer.h>

class PcComm
{
public:
    PcComm(HardwareSerial &hwSerial);
    void loop();


private:
    HardwareSerial &serial;
    std::vector<PcCommand> commands = {
        PcCommand("APN", [this](char *data)
                  { return handleApnCommand(data); }),
        PcCommand("SIM_PIN", [this](char *data)
                  { return handleSimPinCommand(data); }),
        PcCommand("MQTT_SERVER", [this](char *data)
                  { return handleMqttServerCommand(data); }),
        PcCommand("BLE_DEV", [this](char *data)
                  { return handleBleDeviceCommand(data); }),
        PcCommand("BATT_CAL", [this](char *data)
                  { return handleBatteryCalibrationCommand(data); }),
    };
    char rxBuffer[1024];
    uint16_t rxBufferPos = 0u;
    
    SimpleIntervalTimer cyclicInfoInterval = {1000u};

    void parseBuffer();
    bool handleApnCommand(char *data);
    bool handleSimPinCommand(char *data);
    bool handleMqttServerCommand(char *data);
    bool handleBleDeviceCommand(char *data);
    bool handleBatteryCalibrationCommand(char *data);
    
    void sendModemStatus();
};

extern PcComm pcComm;