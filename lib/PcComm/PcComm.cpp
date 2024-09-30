#include "PcComm.h"
#include <IrvineConfiguration.h>
#include <Service.h>
#include <Logger.h>
#include <ModemStatusData.h>
#include <ModemManagement.h>

const char MODULE[] = "PCCOMM";

PcComm pcComm(Serial);

PcComm::PcComm(HardwareSerial &hwSerial) : serial(hwSerial)
{
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

    if (cyclicInfoInterval.check())
    {
        sendModemStatus();
    }
}

void PcComm::sendModemStatus()
{
    ModemStatusData status = modemManagement.getModemStatusData();

    serial.printf("+STATUS: %d,%d,%d,%d,%d,%d,%s,%s,%s,%s,%s,%s,%s,%s,%d,%s\n",
                  status.pinEnabled,
                  status.simReady,
                  status.modemPoweredOn,
                  status.gprsConnected,
                  status.mqttConnected,
                  status.gpsEnabled,
                  status.modemName.c_str(),
                  status.modemModel.c_str(),
                  status.modemImei.c_str(),
                  status.simCcid.c_str(),
                  status.simImsi.c_str(),
                  status.gsmOperator.c_str(),
                  status.gsmNetworkType.c_str(),
                  status.gsmFrequency.c_str(),
                  status.signal,
                  irvineConfiguration.device.deviceId);
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
                bool result = callback(bufPtr);
                if (result)
                {
                    serial.printf("OK\n");
                }
                else
                {
                    serial.printf("ERROR\n");
                }
                return;
            }
        }

        logger.logPrintF(LogSeverity::WARNING, MODULE, "Unknown input command");
    }
}

bool PcComm::handleApnCommand(char *data)
{
    bool result = true;
    if (data[0] == '?')
    {
        serial.printf("+APN: %s,%s,%s\n", irvineConfiguration.modem.apn, irvineConfiguration.modem.apnUsername, irvineConfiguration.modem.apnPassword);
    }
    else if (data[0] == '=')
    {
        char *bufPtr = &data[1u];
        char *apn = strtok(bufPtr, ",");
        char *username = strtok(NULL, ",");
        char *pass = strtok(NULL, "\n\r");

        if (!apn)
            result = false;

        if (!username)
            username = "";
        if (!pass)
            pass = "";

        if (result)
        {
            result &= irvineConfiguration.setParameter("mdm.apn", apn);
            result &= irvineConfiguration.setParameter("mdm.apnUser", username);
            result &= irvineConfiguration.setParameter("mdm.apnPass", pass);
        }
    }
    return result;
}

bool PcComm::handleSimPinCommand(char *data)
{
    bool result = true;
    if (data[0] == '?')
    {
        serial.printf("+SIM_PIN: %s\n", irvineConfiguration.modem.pin);
    }
    else if (data[0] == '=')
    {
        char *bufPtr = &data[1u];

        result = irvineConfiguration.setParameter("mdm.pin", bufPtr);
    }

    return result;
}

bool PcComm::handleMqttServerCommand(char *data)
{
    bool result = true;
    if (data[0] == '?')
    {
        serial.printf("+MQTT_SERVER: %s,%u,%s,%s\n",
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

        if (!server)
            server = "";
        if (!port)
            port = "0";
        if (!username)
            username = "";
        if (!password)
            password = "";

        result &= irvineConfiguration.setParameter("srv.mqttHost", server);
        result &= irvineConfiguration.setParameter("srv.mqttPort", port);
        result &= irvineConfiguration.setParameter("srv.mqttUser", username);
        result &= irvineConfiguration.setParameter("srv.mqttPass", password);
    }
    else
    {
        result = false;
    }

    return result;
}

bool PcComm::handleBleDeviceCommand(char *data)
{
    bool result = true;
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

            serial.printf("+BLE_DEV: %u,%u,%s,%u\n",
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

        if (!dev || !type || !mac || !minInterval)
            result = false;

        if (!dev)
            dev = "";
        if (!type)
            type = "0";
        if (!mac)
            mac = "000000000000";
        if (!minInterval)
            minInterval = "0";

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
    }
    else
    {
        result = false;
    }
    return result;
}

bool PcComm::handleBatteryCalibrationCommand(char *data)
{
    bool result = false;
    if (data[0] == '=')
    {
        char *bufPtr = &data[1u];
        char *ref_voltage = strtok(bufPtr, "\n\r");

        if (ref_voltage)
        {
            float ref_voltage_value = atoff(ref_voltage);
            if (ref_voltage_value != 0.0f)
            {
                result = service.batteryCalibration(ref_voltage_value);
            }
        }
    }
    return result;
}

bool PcComm::handleDeviceCommand(char *data)
{
    bool result = true;
    if (data[0] == '?')
    {
        serial.printf("+DEVICE: %f,%d,%lu,%s,%f\n",
                      irvineConfiguration.device.ignitionVoltageThreshold,
                      irvineConfiguration.device.ignitionOffDelay,
                      irvineConfiguration.device.logSeverity,
                      irvineConfiguration.device.deviceId,
                      irvineConfiguration.device.batteryCalibrationScale);
    }
    else if (data[0] == '=')
    {
        char *bufPtr = &data[1u];
        char *ignTreshold = strtok(bufPtr, ",");
        char *logSev = strtok(NULL, ",");
        char *ignOffDel = strtok(NULL, "\n\r");

        if (!ignTreshold || !logSev || !ignOffDel)
            result = false;
        if (result)
        {
            result &= irvineConfiguration.setParameter("dev.ignVolThr", ignTreshold);
            // result &= irvineConfiguration.setParameter("dev.logSev", logSev);
            result &= irvineConfiguration.setParameter("dev.ignOffDelay", ignOffDel);
        }
    }
    else
    {
        result = false;
    }

    return result;
}
