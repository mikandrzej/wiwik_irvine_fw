
#include "IrvineConfiguration.h"
#include <Preferences.h>
#include <Logger.h>
#include <ArduinoJson.h>
#include <WiFi.h>

const char MODULE[] = "CFG";

IrvineConfiguration irvineConfiguration;

static ConfigurationParameter parameters[] = {
    {"srv.mqttHost", ConfigurationParameterType::STR, &irvineConfiguration.server.mqttHost, sizeof(irvineConfiguration.server.mqttHost)},
    {"srv.mqttPort", ConfigurationParameterType::USHORT, &irvineConfiguration.server.mqttPort, 0},
    {"srv.mqttUser", ConfigurationParameterType::STR, &irvineConfiguration.server.mqttUsername, sizeof(irvineConfiguration.server.mqttUsername)},
    {"srv.mqttPass", ConfigurationParameterType::STR, &irvineConfiguration.server.mqttPassword, sizeof(irvineConfiguration.server.mqttPassword)},
    {"srv.mngHost", ConfigurationParameterType::STR, &irvineConfiguration.server.managementHost, sizeof(irvineConfiguration.server.managementHost)},
    {"srv.mngPort", ConfigurationParameterType::USHORT, &irvineConfiguration.server.managementPort, 0},
    {"srv.mngUser", ConfigurationParameterType::STR, &irvineConfiguration.server.managementUsername, sizeof(irvineConfiguration.server.managementUsername)},
    {"srv.mngPass", ConfigurationParameterType::STR, &irvineConfiguration.server.managementPassword, sizeof(irvineConfiguration.server.managementPassword)},

    {"mdm.apn", ConfigurationParameterType::STR, &irvineConfiguration.modem.apn, sizeof(irvineConfiguration.modem.apn)},
    {"mdm.apnUser", ConfigurationParameterType::STR, &irvineConfiguration.modem.apnUsername, sizeof(irvineConfiguration.modem.apnUsername)},
    {"mdm.apnPass", ConfigurationParameterType::STR, &irvineConfiguration.modem.apnPassword, sizeof(irvineConfiguration.modem.apnPassword)},
    {"mdm.pin", ConfigurationParameterType::STR, &irvineConfiguration.modem.pin, sizeof(irvineConfiguration.modem.pin)},
    {"mdm.natRoam", ConfigurationParameterType::BOOL, &irvineConfiguration.modem.nationalRoaming, 0},
    {"mdm.inatRoam", ConfigurationParameterType::BOOL, &irvineConfiguration.modem.internationalRoaming, sizeof(irvineConfiguration.modem.internationalRoaming)},
    {"mdm.opWlst[0]", ConfigurationParameterType::STR, &irvineConfiguration.modem.operatorsWhitelist[0], sizeof(irvineConfiguration.modem.operatorsWhitelist[0])},
    {"mdm.opWlst[1]", ConfigurationParameterType::STR, &irvineConfiguration.modem.operatorsWhitelist[1], sizeof(irvineConfiguration.modem.operatorsWhitelist[1])},
    {"mdm.opWlst[2]", ConfigurationParameterType::STR, &irvineConfiguration.modem.operatorsWhitelist[2], sizeof(irvineConfiguration.modem.operatorsWhitelist[2])},
    {"mdm.opWlst[3]", ConfigurationParameterType::STR, &irvineConfiguration.modem.operatorsWhitelist[3], sizeof(irvineConfiguration.modem.operatorsWhitelist[3])},
    {"mdm.opWlst[4]", ConfigurationParameterType::STR, &irvineConfiguration.modem.operatorsWhitelist[4], sizeof(irvineConfiguration.modem.operatorsWhitelist[4])},
    {"mdm.opWlst[5]", ConfigurationParameterType::STR, &irvineConfiguration.modem.operatorsWhitelist[5], sizeof(irvineConfiguration.modem.operatorsWhitelist[5])},
    {"mdm.opWlst[6]", ConfigurationParameterType::STR, &irvineConfiguration.modem.operatorsWhitelist[6], sizeof(irvineConfiguration.modem.operatorsWhitelist[6])},
    {"mdm.opWlst[7]", ConfigurationParameterType::STR, &irvineConfiguration.modem.operatorsWhitelist[7], sizeof(irvineConfiguration.modem.operatorsWhitelist[7])},
    {"mdm.opWlst[8]", ConfigurationParameterType::STR, &irvineConfiguration.modem.operatorsWhitelist[8], sizeof(irvineConfiguration.modem.operatorsWhitelist[8])},
    {"mdm.opWlst[9]", ConfigurationParameterType::STR, &irvineConfiguration.modem.operatorsWhitelist[9], sizeof(irvineConfiguration.modem.operatorsWhitelist[9])},
    {"mdm.opWlst[0]", ConfigurationParameterType::STR, &irvineConfiguration.modem.operatorsBlacklist[0], sizeof(irvineConfiguration.modem.operatorsBlacklist[0])},
    {"mdm.opWlst[1]", ConfigurationParameterType::STR, &irvineConfiguration.modem.operatorsBlacklist[1], sizeof(irvineConfiguration.modem.operatorsBlacklist[1])},
    {"mdm.opWlst[2]", ConfigurationParameterType::STR, &irvineConfiguration.modem.operatorsBlacklist[2], sizeof(irvineConfiguration.modem.operatorsBlacklist[2])},
    {"mdm.opWlst[3]", ConfigurationParameterType::STR, &irvineConfiguration.modem.operatorsBlacklist[3], sizeof(irvineConfiguration.modem.operatorsBlacklist[3])},
    {"mdm.opWlst[4]", ConfigurationParameterType::STR, &irvineConfiguration.modem.operatorsBlacklist[4], sizeof(irvineConfiguration.modem.operatorsBlacklist[4])},
    {"mdm.opWlst[5]", ConfigurationParameterType::STR, &irvineConfiguration.modem.operatorsBlacklist[5], sizeof(irvineConfiguration.modem.operatorsBlacklist[5])},
    {"mdm.opWlst[6]", ConfigurationParameterType::STR, &irvineConfiguration.modem.operatorsBlacklist[6], sizeof(irvineConfiguration.modem.operatorsBlacklist[6])},
    {"mdm.opWlst[7]", ConfigurationParameterType::STR, &irvineConfiguration.modem.operatorsBlacklist[7], sizeof(irvineConfiguration.modem.operatorsBlacklist[7])},
    {"mdm.opWlst[8]", ConfigurationParameterType::STR, &irvineConfiguration.modem.operatorsBlacklist[8], sizeof(irvineConfiguration.modem.operatorsBlacklist[8])},
    {"mdm.opWlst[9]", ConfigurationParameterType::STR, &irvineConfiguration.modem.operatorsBlacklist[9], sizeof(irvineConfiguration.modem.operatorsBlacklist[9])},

    {"sms.mngAllNo[0]", ConfigurationParameterType::STR, &irvineConfiguration.sms.mngAllowedNumbers[0], sizeof(irvineConfiguration.sms.mngAllowedNumbers[0])},
    {"sms.mngAllNo[1]", ConfigurationParameterType::STR, &irvineConfiguration.sms.mngAllowedNumbers[1], sizeof(irvineConfiguration.sms.mngAllowedNumbers[1])},
    {"sms.mngAllNo[2]", ConfigurationParameterType::STR, &irvineConfiguration.sms.mngAllowedNumbers[2], sizeof(irvineConfiguration.sms.mngAllowedNumbers[2])},
    {"sms.mngAllNo[3]", ConfigurationParameterType::STR, &irvineConfiguration.sms.mngAllowedNumbers[3], sizeof(irvineConfiguration.sms.mngAllowedNumbers[3])},
    {"sms.mngAllNo[4]", ConfigurationParameterType::STR, &irvineConfiguration.sms.mngAllowedNumbers[4], sizeof(irvineConfiguration.sms.mngAllowedNumbers[4])},

    {"tim.source", ConfigurationParameterType::UCHAR, &irvineConfiguration.time.source, 0},
    {"tim.ntpSrv", ConfigurationParameterType::STR, &irvineConfiguration.time.ntpServer, sizeof(irvineConfiguration.time.ntpServer)},
    {"tim.syncInt", ConfigurationParameterType::UINT, &irvineConfiguration.time.syncInterval, 0},

    {"veh.ignSrc", ConfigurationParameterType::UCHAR, &irvineConfiguration.vehicle.ignitionSource, 0},

    {"gps.minDist", ConfigurationParameterType::UINT, &irvineConfiguration.gps.minimumDistance, 0},
    {"gps.maxInterv", ConfigurationParameterType::UINT, &irvineConfiguration.gps.maxInterval, 0},
    {"gps.slIgnTmt", ConfigurationParameterType::UINT, &irvineConfiguration.gps.sleepAfterIgnitionOffTimeout, 0},
    {"gps.frPosStop", ConfigurationParameterType::UCHAR, &irvineConfiguration.gps.freezePositionDuringStop, 0},
    {"gps.movSpdThr", ConfigurationParameterType::UINT, &irvineConfiguration.gps.movementSpeedThreshold, 0},
    {"gps.movStopDly", ConfigurationParameterType::UINT, &irvineConfiguration.gps.movementStopDelay, 0},
    {"gps.movInterv", ConfigurationParameterType::UINT, &irvineConfiguration.gps.movementLogInterval, 0},
    {"gps.stopInterv", ConfigurationParameterType::UINT, &irvineConfiguration.gps.stopLogInterval, 0},
    {"gps.hPrecOD", ConfigurationParameterType::BOOL, &irvineConfiguration.gps.highPrecisionOnDemand, 0},
    {"gps.hPrecODDur", ConfigurationParameterType::UINT, &irvineConfiguration.gps.highPrecisionOnDemandDuration, 0},
    {"gps.jammDet", ConfigurationParameterType::BOOL, &irvineConfiguration.gps.jammingDetection, 0},

    {"obd.protType", ConfigurationParameterType::UCHAR, &irvineConfiguration.obd.protocolType, 0},

    {"dev.batInterv", ConfigurationParameterType::UINT, &irvineConfiguration.device.batteryInterval, 0},
    {"dev.ignVolThr", ConfigurationParameterType::UINT, &irvineConfiguration.device.ignitionVoltageThreshold, 0},
    {"dev.ignOffDelay", ConfigurationParameterType::UINT, &irvineConfiguration.device.ignitionOffDelay, 0},
    {"dev.movInterv", ConfigurationParameterType::UINT, &irvineConfiguration.device.movementLogInterval, 0},
    {"dev.stopInterv", ConfigurationParameterType::UINT, &irvineConfiguration.device.stopLogInterval, 0},
    {"dev.pwrOutDet", ConfigurationParameterType::BOOL, &irvineConfiguration.device.powerOutageDetection, 0},
    {"dev.privBusRide", ConfigurationParameterType::BOOL, &irvineConfiguration.device.privateBusinessRide, 0},
    {"dev.deviceId", ConfigurationParameterType::STR, &irvineConfiguration.device.deviceId, sizeof(irvineConfiguration.device.deviceId)},
    {"dev.batCalib", ConfigurationParameterType::FLOAT, &irvineConfiguration.device.batteryCalibrationScale, 0},

    {"acc.movThre", ConfigurationParameterType::UINT, &irvineConfiguration.accelerometer.movementThreshold, 0},
    {"acc.movStopDel", ConfigurationParameterType::UINT, &irvineConfiguration.accelerometer.movementStopDelay, 0},
    {"acc.logEna", ConfigurationParameterType::BOOL, &irvineConfiguration.accelerometer.logEnable, 0},
    {"acc.crashDetThr", ConfigurationParameterType::UINT, &irvineConfiguration.accelerometer.crashDetectionThreshold, 0},
    {"acc.towDet", ConfigurationParameterType::BOOL, &irvineConfiguration.accelerometer.towDetection, 0},

    {"bbox.interv", ConfigurationParameterType::UINT, &irvineConfiguration.blackBox.interval, 0},

    {"ble.devMngEn", ConfigurationParameterType::BOOL, &irvineConfiguration.bluetooth.deviceManagementEnable, 0},
    {"ble[0].mac", ConfigurationParameterType::BYTES, &irvineConfiguration.bluetooth.devices[0].macAddress, sizeof(irvineConfiguration.bluetooth.devices[0].macAddress)},
    {"ble[1].mac", ConfigurationParameterType::BYTES, &irvineConfiguration.bluetooth.devices[1].macAddress, sizeof(irvineConfiguration.bluetooth.devices[1].macAddress)},
    {"ble[2].mac", ConfigurationParameterType::BYTES, &irvineConfiguration.bluetooth.devices[2].macAddress, sizeof(irvineConfiguration.bluetooth.devices[2].macAddress)},
    {"ble[3].mac", ConfigurationParameterType::BYTES, &irvineConfiguration.bluetooth.devices[3].macAddress, sizeof(irvineConfiguration.bluetooth.devices[3].macAddress)},
    {"ble[4].mac", ConfigurationParameterType::BYTES, &irvineConfiguration.bluetooth.devices[4].macAddress, sizeof(irvineConfiguration.bluetooth.devices[4].macAddress)},
    {"ble[5].mac", ConfigurationParameterType::BYTES, &irvineConfiguration.bluetooth.devices[5].macAddress, sizeof(irvineConfiguration.bluetooth.devices[5].macAddress)},
    {"ble[6].mac", ConfigurationParameterType::BYTES, &irvineConfiguration.bluetooth.devices[6].macAddress, sizeof(irvineConfiguration.bluetooth.devices[6].macAddress)},
    {"ble[7].mac", ConfigurationParameterType::BYTES, &irvineConfiguration.bluetooth.devices[7].macAddress, sizeof(irvineConfiguration.bluetooth.devices[7].macAddress)},
    {"ble[8].mac", ConfigurationParameterType::BYTES, &irvineConfiguration.bluetooth.devices[8].macAddress, sizeof(irvineConfiguration.bluetooth.devices[8].macAddress)},
    {"ble[9].mac", ConfigurationParameterType::BYTES, &irvineConfiguration.bluetooth.devices[9].macAddress, sizeof(irvineConfiguration.bluetooth.devices[9].macAddress)},
    {"ble[0].type", ConfigurationParameterType::UCHAR, &irvineConfiguration.bluetooth.devices[0].type, 0u},
    {"ble[1].type", ConfigurationParameterType::UCHAR, &irvineConfiguration.bluetooth.devices[1].type, 0u},
    {"ble[2].type", ConfigurationParameterType::UCHAR, &irvineConfiguration.bluetooth.devices[2].type, 0u},
    {"ble[3].type", ConfigurationParameterType::UCHAR, &irvineConfiguration.bluetooth.devices[3].type, 0u},
    {"ble[4].type", ConfigurationParameterType::UCHAR, &irvineConfiguration.bluetooth.devices[4].type, 0u},
    {"ble[5].type", ConfigurationParameterType::UCHAR, &irvineConfiguration.bluetooth.devices[5].type, 0u},
    {"ble[6].type", ConfigurationParameterType::UCHAR, &irvineConfiguration.bluetooth.devices[6].type, 0u},
    {"ble[7].type", ConfigurationParameterType::UCHAR, &irvineConfiguration.bluetooth.devices[7].type, 0u},
    {"ble[8].type", ConfigurationParameterType::UCHAR, &irvineConfiguration.bluetooth.devices[8].type, 0u},
    {"ble[9].type", ConfigurationParameterType::UCHAR, &irvineConfiguration.bluetooth.devices[9].type, 0u},
    {"ble[0].minIval", ConfigurationParameterType::UINT, &irvineConfiguration.bluetooth.devices[0].minInterval, 0u},
    {"ble[1].minIval", ConfigurationParameterType::UINT, &irvineConfiguration.bluetooth.devices[1].minInterval, 0u},
    {"ble[2].minIval", ConfigurationParameterType::UINT, &irvineConfiguration.bluetooth.devices[2].minInterval, 0u},
    {"ble[3].minIval", ConfigurationParameterType::UINT, &irvineConfiguration.bluetooth.devices[3].minInterval, 0u},
    {"ble[4].minIval", ConfigurationParameterType::UINT, &irvineConfiguration.bluetooth.devices[4].minInterval, 0u},
    {"ble[5].minIval", ConfigurationParameterType::UINT, &irvineConfiguration.bluetooth.devices[5].minInterval, 0u},
    {"ble[6].minIval", ConfigurationParameterType::UINT, &irvineConfiguration.bluetooth.devices[6].minInterval, 0u},
    {"ble[7].minIval", ConfigurationParameterType::UINT, &irvineConfiguration.bluetooth.devices[7].minInterval, 0u},
    {"ble[8].minIval", ConfigurationParameterType::UINT, &irvineConfiguration.bluetooth.devices[8].minInterval, 0u},
    {"ble[9].minIval", ConfigurationParameterType::UINT, &irvineConfiguration.bluetooth.devices[9].minInterval, 0u},
};

bool IrvineConfiguration::begin()
{
    if (!preferences.begin("irvine_cfg"))
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "configuration begin fail");
        return false;
    }

    for (auto &parameter : parameters)
    {
        loadParameter(parameter);
    }

    bluetooth.devices[0].macAddress[0] = 0xE7;
    bluetooth.devices[0].macAddress[1] = 0xD1;
    bluetooth.devices[0].macAddress[2] = 0x84;
    bluetooth.devices[0].macAddress[3] = 0x91;
    bluetooth.devices[0].macAddress[4] = 0x15;
    bluetooth.devices[0].macAddress[5] = 0x78;
    bluetooth.devices[0].type = BluetoothDeviceType::JAALEE_SENSOR;
    bluetooth.devices[0].minInterval = 10000u;

    bluetooth.devices[1].macAddress[0] = 0xE5;
    bluetooth.devices[1].macAddress[1] = 0x8B;
    bluetooth.devices[1].macAddress[2] = 0x12;
    bluetooth.devices[1].macAddress[3] = 0xEC;
    bluetooth.devices[1].macAddress[4] = 0xC3;
    bluetooth.devices[1].macAddress[5] = 0xA8;
    bluetooth.devices[1].type = BluetoothDeviceType::JAALEE_SENSOR;
    bluetooth.devices[1].minInterval = 10000u;

    bluetooth.devices[2].macAddress[0] = 0xE8;
    bluetooth.devices[2].macAddress[1] = 0xD0;
    bluetooth.devices[2].macAddress[2] = 0x4B;
    bluetooth.devices[2].macAddress[3] = 0x01;
    bluetooth.devices[2].macAddress[4] = 0xA6;
    bluetooth.devices[2].macAddress[5] = 0xDC;
    bluetooth.devices[2].type = BluetoothDeviceType::M52PAS_SENSOR;
    bluetooth.devices[2].minInterval = 10000u;

    bluetooth.devices[3].macAddress[0] = 0xF2;
    bluetooth.devices[3].macAddress[1] = 0x83;
    bluetooth.devices[3].macAddress[2] = 0x23;
    bluetooth.devices[3].macAddress[3] = 0xF8;
    bluetooth.devices[3].macAddress[4] = 0x6A;
    bluetooth.devices[3].macAddress[5] = 0x9D;
    bluetooth.devices[3].type = BluetoothDeviceType::M52PAS_SENSOR;
    bluetooth.devices[3].minInterval = 10000u;

    bluetooth.devices[4].macAddress[0] = 0xF7;
    bluetooth.devices[4].macAddress[1] = 0xAE;
    bluetooth.devices[4].macAddress[2] = 0x34;
    bluetooth.devices[4].macAddress[3] = 0xE2;
    bluetooth.devices[4].macAddress[4] = 0xA5;
    bluetooth.devices[4].macAddress[5] = 0xE7;
    bluetooth.devices[4].type = BluetoothDeviceType::M52PAS_SENSOR;
    bluetooth.devices[4].minInterval = 10000u;

    bluetooth.devices[5].macAddress[0] = 0xE8;
    bluetooth.devices[5].macAddress[1] = 0xE6;
    bluetooth.devices[5].macAddress[2] = 0x37;
    bluetooth.devices[5].macAddress[3] = 0x56;
    bluetooth.devices[5].macAddress[4] = 0xD5;
    bluetooth.devices[5].macAddress[5] = 0xD2;
    bluetooth.devices[5].type = BluetoothDeviceType::JAALEE_SENSOR;
    bluetooth.devices[5].minInterval = 10000u;

    uint8_t mac[6];
    WiFi.macAddress(mac);
    sprintf(device.deviceId, "irvine_%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    device.batteryInterval = 60000u;

    sprintf(server.mqttHost, "mqtt.7frost.com");
    server.mqttPort = 1883u;
    sprintf(server.mqttUsername, "root");
    sprintf(server.mqttPassword, "password");

    sprintf(modem.pin, "4967");
    sprintf(modem.apn, "playmetric");
    sprintf(modem.apnUsername, "");
    sprintf(modem.apnPassword, "");

    gps.maxInterval = 60000u;
    gps.minimumDistance = 20u;
    gps.stopLogInterval = 60000u;
    gps.movementLogInterval = 10000u;
    gps.minimumDistance = 100u;

    obd.speedActive = true;
    obd.speedInterval = 1000u;
    obd.engineSpeedActive = true;
    obd.engineSpeedInterval = 1000u;
    obd.fuelLevelActive = true;
    obd.fuelLevelInterval = 1000u;
    obd.protocolType = ObdProtocolType::AA_AT_THE_END;

    vehicle.stopLogInterval = 60000u;
    vehicle.movementLogInterval = 5000u;
    vehicle.ignitionSource = VehicleIgnitionSource::VOLTAGE;

    time.source = TimeSource::GSM;

    printConfiguration();

    return true;
}

void IrvineConfiguration::printConfiguration()
{
    Serial.print("Device configuration:\r\n");
    for (auto &parameter : parameters)
    {
        printParameter(parameter);
    }
}

bool IrvineConfiguration::setParameter(const char *param, const char *value)
{
    for (auto &parameter : parameters)
    {
        if (0 == strcmp(param, parameter.name))
        {
            logger.logPrintF(LogSeverity::INFO, MODULE, "setParameter: %s : %s", param, value);

            switch (parameter.type)
            {
            case ConfigurationParameterType::BYTES:
            {
                uint8_t bytes[parameter.len];
                for (int i = 0; i < parameter.len; i++)
                {
                    sscanf(&value[i * 2], "%02hhx", &bytes[i]);
                }
                return preferences.putBytes(parameter.name, bytes, parameter.len) == parameter.len;
            }

            case ConfigurationParameterType::STR:
                return preferences.putBytes(parameter.name, value, parameter.len) == parameter.len;

            case ConfigurationParameterType::UCHAR:
            {
                uint8_t val;
                if (1 != sscanf(value, "%hhu", &val))
                {
                    logger.logPrintF(LogSeverity::WARNING, MODULE, "Failed to parse UCHAR parameter");
                    return false;
                }
                return preferences.putUChar(parameter.name, val) == 1;
            }

            case ConfigurationParameterType::USHORT:
            {
                uint16_t val;
                if (1 != sscanf(value, "%hu", &val))
                {
                    logger.logPrintF(LogSeverity::WARNING, MODULE, "Failed to parse USHORT parameter");
                    return false;
                }
                return preferences.putUShort(parameter.name, val) == 2;
            }

            case ConfigurationParameterType::UINT:
            {
                uint32_t val;
                if (1 != sscanf(value, "%u", &val))
                {
                    logger.logPrintF(LogSeverity::WARNING, MODULE, "Failed to parse UINT parameter");
                    return false;
                }
                return preferences.putUInt(parameter.name, val) == 4;
            }

            case ConfigurationParameterType::BOOL:
            {
                bool val;
                if (1 != sscanf(value, "%hhu", &val))
                {
                    logger.logPrintF(LogSeverity::WARNING, MODULE, "Failed to parse BOOL parameter");
                    return false;
                }
                return preferences.putBool(parameter.name, val) == 1;
            }
            case ConfigurationParameterType::FLOAT:
            {
                float val;
                if (1 != sscanf(value, "%f", &val))
                {
                    logger.logPrintF(LogSeverity::WARNING, MODULE, "Failed to parse FLOAT parameter");
                    return false;
                }
                return preferences.putFloat(parameter.name, val) == 1;
            }
            default:
                logger.logPrintF(LogSeverity::WARNING, MODULE, "Invalid parameter type (%d) %s", parameter.type, param);
                return false;
            }
        }
    }

    logger.logPrintF(LogSeverity::WARNING, MODULE, "parameter not found: %s : %s", param, value);

    return false;
}

bool IrvineConfiguration::setParameter(const char *param, const float value)
{
    for (auto &parameter : parameters)
    {
        if (0 == strcmp(param, parameter.name))
        {
            logger.logPrintF(LogSeverity::INFO, MODULE, "setParameter: %s : %f", param, value);

            if (parameter.type == ConfigurationParameterType::FLOAT)
            {
                if (preferences.putFloat(parameter.name, value) == 4)
                {
                    *((float *)parameter.pointer) = value;
                    logger.logPrintF(LogSeverity::DEBUG, MODULE, "Parameter %s set %f completed", parameter.name, param);
                    return true;
                }
                else
                {
                    logger.logPrintF(LogSeverity::DEBUG, MODULE, "Parameter %s set %f FAILED", parameter.name, param);
                    return false;
                }
            }
            else
            {
                logger.logPrintF(LogSeverity::WARNING, MODULE, "Invalid parameter type (%d) %s", parameter.type, param);
                return false;
            }
        }
    }

    logger.logPrintF(LogSeverity::WARNING, MODULE, "parameter not found: %s : %s", param, value);
    return false;
}

void IrvineConfiguration::checkDefaultConfiguration(void)
{
}

void IrvineConfiguration::printParameter(ConfigurationParameter &parameter)
{
    Serial.print(parameter.name);
    Serial.print(" = ");

    switch (parameter.type)
    {
    case ConfigurationParameterType::STR:
        Serial.print("\"");
        Serial.print((const char *)parameter.pointer);
        Serial.print("\"");
        break;

    case ConfigurationParameterType::BYTES:
        for (int k = 0; k < parameter.len; k++)
        {
            Serial.printf("%02X", ((uint8_t *)parameter.pointer)[k]);
        }
        break;

    case ConfigurationParameterType::UCHAR:
        Serial.print(*((uint8_t *)parameter.pointer));
        break;

    case ConfigurationParameterType::USHORT:
        Serial.print(*((uint16_t *)parameter.pointer));
        break;

    case ConfigurationParameterType::UINT:
        Serial.print(*((uint32_t *)parameter.pointer));
        break;

    case ConfigurationParameterType::BOOL:
        Serial.print(*((bool *)parameter.pointer) ? "true" : "false");
        break;

    case ConfigurationParameterType::FLOAT:
        Serial.print(*((float *)parameter.pointer));
        break;
    }

    Serial.print("\r\n");
}

void IrvineConfiguration::loadParameter(ConfigurationParameter &parameter)
{
    switch (parameter.type)
    {
    case ConfigurationParameterType::STR:
        preferences.getBytes(parameter.name, parameter.pointer, parameter.len);
        break;

    case ConfigurationParameterType::BYTES:
        preferences.getBytes(parameter.name, parameter.pointer, parameter.len);
        break;

    case ConfigurationParameterType::UCHAR:
        *((uint8_t *)parameter.pointer) = preferences.getUChar(parameter.name);
        break;

    case ConfigurationParameterType::USHORT:
        *((uint16_t *)parameter.pointer) = preferences.getUShort(parameter.name);
        break;

    case ConfigurationParameterType::UINT:
        *((uint32_t *)parameter.pointer) = preferences.getUInt(parameter.name);
        break;

    case ConfigurationParameterType::BOOL:
        *((bool *)parameter.pointer) = preferences.getBool(parameter.name);
        break;

    case ConfigurationParameterType::FLOAT:
        *((float *)parameter.pointer) = preferences.getFloat(parameter.name);
        break;
    }
}