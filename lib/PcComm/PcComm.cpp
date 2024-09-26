#include "PcComm.h"
#include <IrvineConfiguration.h>
#include <Logger.h>

const char MODULE[] = "PCCOMM";

PcComm pcComm(Serial);

PcComm::PcComm(HardwareSerial &hwSerial) : serial(hwSerial)
{
    // commands.push_back(new PcCommand("APN", [this](char *data)
    //                                  { handleApnCommand(data); }));
    // commands.push_back(new PcCommand("SIM_PIN", [this](char *data)
    //                                  { handleSimPinCommand(data); }));
    // commands.push_back(new PcCommand("MQTT_SERVER", [this](char *data)
    //                                  { handleMqttServerCommand(data); }));
    // commands.push_back(new PcCommand("BLE_DEV", [this](char *data)
    //                                  { handleBleDeviceCommand(data); }));
}

void PcComm::loop()
{
    while (serial.available())
    {
        char newChar = serial.read();

        if (newChar == '\n' || newChar == '\r')
        {
            if (rxBufferPos == 0)
                continue;

            rxBuffer[rxBufferPos] = '\0';
            parseBuffer();

            rxBufferPos = 0u;
        }
        else
        {
            rxBuffer[rxBufferPos++] = newChar;
        }
    }
}

void PcComm::parseBuffer()
{
    if (0 == strncmp(rxBuffer, "AT+", 3))
    {
        char *bufPtr = &rxBuffer[3];

        for (auto &_command : commands)
        {
            if (0 == strncmp(bufPtr, _command.getCommand(), _command.getCommandLen()))
            {
                bufPtr += _command.getCommandLen();
                auto callback = _command.getCallback();
                callback(bufPtr);
                return;
            }
        }

        logger.logPrintF(LogSeverity::WARNING, MODULE, "Unknown input command");
    }
}

void PcComm::handleApnCommand(char *data)
{
    if (data[0] == '?')
    {
        serial.printf("+APN=%s,%s,%s\n", irvineConfiguration.modem.apn, irvineConfiguration.modem.apnUsername, irvineConfiguration.modem.apnPassword);
    }
    else if (data[0] == '=')
    {
        char *bufPtr = &data[1u];
        char *apn = strtok(bufPtr, ",");
        char *username = strtok(NULL, ",");
        char *pass = strtok(NULL, "\n\r");

        bool result = true;

        if (!apn || !username || !pass)
            result = false;

        if (result)
        {
            result &= irvineConfiguration.setParameter("mdm.apn", apn);
            result &= irvineConfiguration.setParameter("mdm.apnUser", username);
            result &= irvineConfiguration.setParameter("mdm.apnPass", pass);
        }

        if (result)
        {
            serial.printf("OK\n");
        }
        else
        {
            serial.printf("ERROR\n");
        }
    }
}

void PcComm::handleSimPinCommand(char *data)
{
    if (data[0] == '?')
    {
        serial.printf("+SIM_PIN=%s\n", irvineConfiguration.modem.pin);
    }
    else if (data[0] == '=')
    {
        char *bufPtr = &data[1u];

        bool result = irvineConfiguration.setParameter("mdm.pin", bufPtr);

        if (result)
            serial.printf("OK\n");
        else
            serial.printf("ERROR\n");
    }
}

void PcComm::handleMqttServerCommand(char *data)
{
    if (data[0] == '?')
    {
        serial.printf("+MQTT_SERVER=%s,%u,%s,%s\n",
                      irvineConfiguration.server.mqttHost,
                      irvineConfiguration.server.mqttPort,
                      irvineConfiguration.server.mqttUsername,
                      irvineConfiguration.server.mqttPassword);
    }
    else if (data[0] == '=')
    {
        char *bufPtr = &data[1u];
        char *server = strtok(bufPtr, ",");
        char *port = strtok(NULL, ",");
        char *username = strtok(NULL, ",");
        char *password = strtok(NULL, "\n\r");

        bool result = true;

        if (!port || !username || !password)
            result = false;

        if (result)
        {
            result &= irvineConfiguration.setParameter("srv.mqttHost", server);
            result &= irvineConfiguration.setParameter("srv.mqttPort", port);
            result &= irvineConfiguration.setParameter("srv.mqttUser", username);
            result &= irvineConfiguration.setParameter("srv.mqttPass", password);
        }

        if (result)
            serial.printf("OK\n");
        else
            serial.printf("ERROR\n");
    }
}

void PcComm::handleBleDeviceCommand(char *data)
{
    if (data[0] == '?')
    {
        for (uint16_t k = 0u; k < MAX_BLUETOOTH_DEVICES; k++)
        {
            auto *device = &irvineConfiguration.bluetooth.devices[k];
            char mac[20];
            sprintf(mac, "%02X%02X%02X%02X%02X%02X",
                    device->macAddress[0],
                    device->macAddress[1],
                    device->macAddress[2],
                    device->macAddress[3],
                    device->macAddress[4],
                    device->macAddress[5]);

            serial.printf("+BLE_DEV=%u,%u,%s,%u\n",
                          k,
                          static_cast<uint32_t>(device->type),
                          mac,
                          device->minInterval);
        }
    }
    else if (data[0] == '=')
    {
        char *bufPtr = &data[1u];
        char *dev = strtok(bufPtr, ",");
        char *type = strtok(NULL, ",");
        char *mac = strtok(NULL, ",");
        char *minInterval = strtok(NULL, "\n\r");

        bool result = true;

        if (!dev || !type || !mac || !minInterval)
            result = false;

        if (result)
        {
            char param[20];
            snprintf(param, 20, "ble[%s].type", dev);
            result &= irvineConfiguration.setParameter(param, type);
            snprintf(param, 20, "ble[%s].mac", dev);
            result &= irvineConfiguration.setParameter(param, mac);
            snprintf(param, 20, "ble[%s].minIval", dev);
            result &= irvineConfiguration.setParameter(param, minInterval);
        }

        if (result)
            serial.printf("OK\n");
        else
            serial.printf("ERROR\n");
    }
}
