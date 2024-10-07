#include "PcComm.h"
#include <IrvineConfiguration.h>
#include <Service.h>
#include <Logger.h>
#include <ModemStatusData.h>
#include <ModemManagement.h>
#include <Device.h>
#include <Vehicle.h>

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
            break;
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

    serial.printf("+STATUS: %d,%d,%d,%d,%d,%d,%s,%s,%s,%s,%s,%s,%s,%s,%d,%s,%lu,%llu,%llu,%llu,%.2f\n",
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
                  irvineConfiguration.device.deviceId,
                  millis(),
                  device.getGpsUnixTimestamp(),
                  device.getGsmUnixTimestamp(),
                  device.getNtpUnixTimestamp(),
                  vehicle.getVccVoltage());
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
        serial.printf("+DEVICE: %lu,%lu,%lu,%s,%f\n",
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

bool PcComm::handleGpsCfgCommand(char *data)
{
    bool result = true;
    if (data[0] == '?')
    {
        serial.printf("+GPS_CFG: %lu,%lu,%lu,%d,%lu,%lu,%lu,%lu,%d,%lu,%d\n",
                      irvineConfiguration.gps.minimumDistance,
                      irvineConfiguration.gps.maxInterval,
                      irvineConfiguration.gps.sleepAfterIgnitionOffTimeout,
                      irvineConfiguration.gps.freezePositionDuringStop,
                      irvineConfiguration.gps.movementSpeedThreshold,
                      irvineConfiguration.gps.movementStopDelay,
                      irvineConfiguration.gps.movementLogInterval,
                      irvineConfiguration.gps.stopLogInterval,
                      irvineConfiguration.gps.highPrecisionOnDemand,
                      irvineConfiguration.gps.highPrecisionOnDemandDuration,
                      irvineConfiguration.gps.jammingDetection);
    }
    else if (data[0] == '=')
    {
        char *bufPtr = &data[1u];
        const char *minimumDistance = strtok(bufPtr, ",");
        const char *maxInterval = strtok(NULL, ",");
        const char *sleepAfterIgnitionOffTimeout = strtok(NULL, ",");
        const char *freezePositionDuringStop = strtok(NULL, ",");
        const char *movementSpeedThreshold = strtok(NULL, ",");
        const char *movementStopDelay = strtok(NULL, ",");
        const char *movementLogInterval = strtok(NULL, ",");
        const char *stopLogInterval = strtok(NULL, ",");
        const char *highPrecisionOnDemand = strtok(NULL, ",");
        const char *highPrecisionOnDemandDuration = strtok(NULL, ",");
        const char *jammingDetection = strtok(NULL, "\n\r");

        minimumDistance = minimumDistance ? minimumDistance : "";
        maxInterval = maxInterval ? maxInterval : "";
        sleepAfterIgnitionOffTimeout = sleepAfterIgnitionOffTimeout ? sleepAfterIgnitionOffTimeout : "";
        freezePositionDuringStop = freezePositionDuringStop ? freezePositionDuringStop : "";
        movementSpeedThreshold = movementSpeedThreshold ? movementSpeedThreshold : "";
        movementStopDelay = movementStopDelay ? movementStopDelay : "";
        movementLogInterval = movementLogInterval ? movementLogInterval : "";
        stopLogInterval = stopLogInterval ? stopLogInterval : "";
        highPrecisionOnDemand = highPrecisionOnDemand ? highPrecisionOnDemand : "";
        highPrecisionOnDemandDuration = highPrecisionOnDemandDuration ? highPrecisionOnDemandDuration : "";
        jammingDetection = jammingDetection ? jammingDetection : "";

        if (result)
        {
            result &= irvineConfiguration.setParameter("gps.minDist", minimumDistance);
            result &= irvineConfiguration.setParameter("gps.maxInterv", maxInterval);
            result &= irvineConfiguration.setParameter("gps.slIgnTmt", sleepAfterIgnitionOffTimeout);
            result &= irvineConfiguration.setParameter("gps.frPosStop", freezePositionDuringStop);
            result &= irvineConfiguration.setParameter("gps.movSpdThr", movementSpeedThreshold);
            result &= irvineConfiguration.setParameter("gps.movStopDly", movementStopDelay);
            result &= irvineConfiguration.setParameter("gps.movInterv", movementLogInterval);
            result &= irvineConfiguration.setParameter("gps.stopInterv", stopLogInterval);
            result &= irvineConfiguration.setParameter("gps.hPrecOD", highPrecisionOnDemand);
            result &= irvineConfiguration.setParameter("gps.hPrecODDur", highPrecisionOnDemandDuration);
            result &= irvineConfiguration.setParameter("gps.jammDet", jammingDetection);
        }
    }
    else
    {
        result = false;
    }

    return result;
}

bool PcComm::handleLoginCommand(char *data)
{
    bool result = false;
    if (data[0] == '=')
    {
        char *bufPtr = &data[1u];
        char *password = strtok(bufPtr, "\n\r");

        if (!password)
            password = "";
        if (0 == strcmp(password, irvineConfiguration.device.password))
        {
            loggedIn = true;
            result = true;
        }
    }

    return result;
}
