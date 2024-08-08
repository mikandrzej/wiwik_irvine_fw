#include "IrvineConfiguration.h"

#include <Preferences.h>
#include <Logger.h>
#include <ArduinoJson.h>
#include <WiFi.h>

const char MODULE[] = "CFG";

IrvineConfiguration irvineConfiguration;

bool IrvineConfiguration::begin()
{
    if (!preferences.begin("irvine_cfg"))
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "configuration begin fail");
        return false;
    }

    loadServerConfig();
    loadModemConfig();
    loadSmsConfig();
    loadTimeConfig();
    loadVehicleConfig();
    loadGpsConfig();
    loadObdConfig();
    loadDeviceConfig();
    loadAccelerometerConfig();
    loadBlackBoxConfig();
    loadBluetoothConfig();

    bluetooth.devices[0].macAddress[0] = 0xE7;
    bluetooth.devices[0].macAddress[1] = 0xD1;
    bluetooth.devices[0].macAddress[2] = 0x84;
    bluetooth.devices[0].macAddress[3] = 0x91;
    bluetooth.devices[0].macAddress[4] = 0x15;
    bluetooth.devices[0].macAddress[5] = 0x78;

    bluetooth.devices[0].type = BluetoothDeviceType::JAALEE_SENSOR;
    bluetooth.devices[0].maxInterval = 10000u;

    bluetooth.devices[1].macAddress[0] = 0xE5;
    bluetooth.devices[1].macAddress[1] = 0x8B;
    bluetooth.devices[1].macAddress[2] = 0x12;
    bluetooth.devices[1].macAddress[3] = 0xEC;
    bluetooth.devices[1].macAddress[4] = 0xC3;
    bluetooth.devices[1].macAddress[5] = 0xA8;
    
    bluetooth.devices[1].type = BluetoothDeviceType::JAALEE_SENSOR;
    bluetooth.devices[1].maxInterval = 10000u;

    uint8_t mac[6];
    WiFi.macAddress(mac);
    sprintf(device.deviceId, "irvine_%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    device.batteryInterval = 60000u;

    sprintf(server.mqttHost, "mqtt.7frost.com");
    server.mqttPort = 1883u;
    sprintf(server.mqttUsername, "root");
    sprintf(server.mqttPassword, "password");

    modem.reportInterval = 10000u;
    sprintf(modem.pin, "4967");
    sprintf(modem.apn, "internet");
    sprintf(modem.apnUsername, "internet");
    sprintf(modem.apnPassword, "internet");

    gps.maxInterval = 60000u;
    gps.minimumDistance = 100u;

    printConfiguration();

    return true;
}

void IrvineConfiguration::loadServerConfig()
{
    preferences.getBytes("server.mqttHost", server.mqttHost, sizeof(server.mqttHost));
    server.mqttPort = preferences.getUShort("server.mqttPort");
    preferences.getBytes("server.mqttUsername", server.mqttUsername, sizeof(server.mqttUsername));
    preferences.getBytes("server.mqttPassword", server.mqttPassword, sizeof(server.mqttPassword));
    preferences.getBytes("server.managementHost", server.managementHost, sizeof(server.managementHost));
    server.managementPort = preferences.getUShort("server.managementPort");
    preferences.getBytes("server.managementUsername", server.managementUsername, sizeof(server.managementUsername));
    preferences.getBytes("server.managementPassword", server.managementPassword, sizeof(server.managementPassword));
}

void IrvineConfiguration::loadModemConfig()
{
    preferences.getBytes("modem.apn", modem.apn, sizeof(modem.apn));
    preferences.getBytes("modem.apnUsername", modem.apnUsername, sizeof(modem.apnUsername));
    preferences.getBytes("modem.apnPassword", modem.apnPassword, sizeof(modem.apnPassword));
    preferences.getBytes("modem.pin", modem.pin, sizeof(modem.pin));
    modem.nationalRoaming = preferences.getBool("modem.nationalRoaming");
    modem.internationalRoaming = preferences.getBool("modem.internationalRoaming");

    for (uint8_t k = 0u; k < MODEM_OPERATORS_WHITELIST_LEN; k++)
    {
        char key[50u];
        sprintf(key, "modem.operatorWhitelist[%u]", k);
        preferences.getBytes(key, modem.operatorsWhitelist[k], sizeof(modem.operatorsWhitelist[k]));
    }
    for (uint8_t k = 0u; k < MODEM_OPERATORS_BLACKLIST_LEN; k++)
    {
        char key[50u];
        sprintf(key, "modem.operatorBlacklist[%u]", k);
        preferences.getBytes(key, modem.operatorsBlacklist[k], sizeof(modem.operatorsBlacklist[k]));
    }
}

void IrvineConfiguration::loadSmsConfig()
{
    for (uint8_t k = 0u; k < SMS_MNG_ALLOWED_NUMBERS_LEN; k++)
    {
        char key[50u];
        sprintf(key, "sms.mngAllowedNumber[%u]", k);
        preferences.getBytes(key, sms.mngAllowedNumbers[k], sizeof(sms.mngAllowedNumbers[k]));
    }
}

void IrvineConfiguration::loadTimeConfig()
{
    time.source = static_cast<TimeSource>(preferences.getUChar("time.source"));
    preferences.getBytes("time.ntpServer", time.ntpServer, sizeof(time.ntpServer));
    time.syncInterval = preferences.getUInt("time.syncInterval");
}

void IrvineConfiguration::loadVehicleConfig()
{
    // Add specific loading instructions for each vehicle parameter
    // Placeholder example for ignition source
    vehicle.ignitionSource = preferences.getUChar("vehicle.ignitionSource");
    // Add the rest of the parameters similarly
}

void IrvineConfiguration::loadGpsConfig()
{
    gps.minimumDistance = preferences.getUInt("gps.minimumDistance");
    gps.maxInterval = preferences.getUInt("gps.maxInterval");
    gps.sleepAfterIgnitionOffTimeout = preferences.getUInt("gps.sleepAfterIgnitionOffTimeout");
    gps.freezePositionDuringStop = preferences.getUChar("gps.freezePositionDuringStop");
    gps.movementSpeedThreshold = preferences.getUInt("gps.movementSpeedThreshold");
    gps.movementStopDelay = preferences.getUInt("gps.movementStopDelay");
    gps.movementLogInterval = preferences.getUInt("gps.movementLogInterval");
    gps.stopLogInterval = preferences.getUInt("gps.stopLogInterval");
    gps.highPrecisionOnDemand = preferences.getBool("gps.highPrecisionOnDemand");
    gps.highPrecisionOnDemandDuration = preferences.getUInt("gps.highPrecisionOnDemandDuration");
    gps.jammingDetection = preferences.getBool("gps.jammingDetection");
    // Geofence and other parameters can be loaded similarly
}

void IrvineConfiguration::loadObdConfig()
{
    // Add specific loading instructions for each OBD parameter
    // Placeholder example for protocol
    obd.protocol = preferences.getUChar("obd.protocol");
    // Add the rest of the parameters similarly
}

void IrvineConfiguration::loadDeviceConfig()
{
    device.batteryInterval = preferences.getUInt("device.batteryInterval");
    device.ignitionVoltageThreshold = preferences.getUInt("device.ignitionVoltageThreshold");
    device.ignitionOffDelay = preferences.getUInt("device.ignitionOffDelay");
    device.movementLogInterval = preferences.getUInt("device.movementLogInterval");
    device.stopLogInterval = preferences.getUInt("device.stopLogInterval");
    device.powerOutageDetection = preferences.getBool("device.powerOutageDetection");
    device.privateBusinessRide = preferences.getBool("device.privateBusinessRide");
    preferences.getBytes("device.deviceId", device.deviceId, sizeof(device.deviceId));
}

void IrvineConfiguration::loadAccelerometerConfig()
{
    accelerometer.movementThreshold = preferences.getUInt("accelerometer.movementThreshold");
    accelerometer.movementStopDelay = preferences.getUInt("accelerometer.movementStopDelay");
    accelerometer.logEnable = preferences.getBool("accelerometer.logEnable");
    accelerometer.crashDetectionThreshold = preferences.getUInt("accelerometer.crashDetectionThreshold");
    accelerometer.towDetection = preferences.getBool("accelerometer.towDetection");
}

void IrvineConfiguration::loadBlackBoxConfig()
{
    blackBox.interval = preferences.getUInt("blackBox.interval");
    // Add specific loading instructions for blackBox parameters
}

void IrvineConfiguration::loadBluetoothConfig()
{
    // Add specific loading instructions for Bluetooth configuration
    // Placeholder example
    bluetooth.deviceManagementEnable = preferences.getBool("bluetooth.deviceManagementEnable");

    // Loop through and load each Bluetooth device, if any
}

void IrvineConfiguration::printConfiguration()
{
    StaticJsonDocument<4096> doc;

    JsonObject serverObj = doc.createNestedObject("server");
    serverObj["mqttHost"] = server.mqttHost;
    serverObj["mqttPort"] = server.mqttPort;
    serverObj["mqttUsername"] = server.mqttUsername;
    serverObj["mqttPassword"] = server.mqttPassword;
    serverObj["managementHost"] = server.managementHost;
    serverObj["managementPort"] = server.managementPort;
    serverObj["managementUsername"] = server.managementUsername;
    serverObj["managementPassword"] = server.managementPassword;

    JsonObject modemObj = doc.createNestedObject("modem");
    modemObj["apn"] = modem.apn;
    modemObj["apnUsername"] = modem.apnUsername;
    modemObj["apnPassword"] = modem.apnPassword;
    modemObj["pin"] = modem.pin;
    modemObj["nationalRoaming"] = modem.nationalRoaming;
    modemObj["internationalRoaming"] = modem.internationalRoaming;

    JsonArray whitelist = modemObj.createNestedArray("operatorsWhitelist");
    for (uint8_t i = 0; i < MODEM_OPERATORS_WHITELIST_LEN; ++i)
    {
        whitelist.add(modem.operatorsWhitelist[i]);
    }

    JsonArray blacklist = modemObj.createNestedArray("operatorsBlacklist");
    for (uint8_t i = 0; i < MODEM_OPERATORS_BLACKLIST_LEN; ++i)
    {
        blacklist.add(modem.operatorsBlacklist[i]);
    }

    JsonObject smsObj = doc.createNestedObject("sms");
    JsonArray allowedNumbers = smsObj.createNestedArray("managementAllowedNumbers");
    for (uint8_t i = 0; i < SMS_MNG_ALLOWED_NUMBERS_LEN; ++i)
    {
        allowedNumbers.add(sms.mngAllowedNumbers[i]);
    }

    JsonObject timeObj = doc.createNestedObject("time");
    timeObj["source"] = static_cast<int>(time.source);
    timeObj["ntpServer"] = time.ntpServer;
    timeObj["syncInterval"] = time.syncInterval;

    JsonObject vehicleObj = doc.createNestedObject("vehicle");
    vehicleObj["ignitionSource"] = vehicle.ignitionSource;
    vehicleObj["movementDetectionSource"] = vehicle.movementDetectionSource;
    vehicleObj["speedSource"] = vehicle.speedSource;
    vehicleObj["movementLogInterval"] = vehicle.movementLogInterval;
    vehicleObj["stopLogInterval"] = vehicle.stopLogInterval;
    vehicleObj["fuelLeakDetectionThreshold"] = vehicle.fuelLeakDetectionThreshold;
    vehicleObj["speedInconsistentDetection"] = vehicle.speedInconsistentDetection;
    vehicleObj["ignitionLock"] = vehicle.ignitionLock;

    JsonObject gpsObj = doc.createNestedObject("gps");
    gpsObj["minimumDistance"] = gps.minimumDistance;
    gpsObj["maxInterval"] = gps.maxInterval;
    gpsObj["sleepAfterIgnitionOffTimeout"] = gps.sleepAfterIgnitionOffTimeout;
    gpsObj["freezePositionDuringStop"] = gps.freezePositionDuringStop;
    gpsObj["movementSpeedThreshold"] = gps.movementSpeedThreshold;
    gpsObj["movementStopDelay"] = gps.movementStopDelay;
    gpsObj["movementLogInterval"] = gps.movementLogInterval;
    gpsObj["stopLogInterval"] = gps.stopLogInterval;
    gpsObj["highPrecisionOnDemand"] = gps.highPrecisionOnDemand;
    gpsObj["highPrecisionOnDemandDuration"] = gps.highPrecisionOnDemandDuration;
    gpsObj["jammingDetection"] = gps.jammingDetection;

    JsonObject obdObj = doc.createNestedObject("obd");
    obdObj["protocol"] = obd.protocol;
    obdObj["speed"] = obd.speed;
    obdObj["engineSpeed"] = obd.engineSpeed;
    obdObj["voltage"] = obd.voltage;
    obdObj["fuelLevel"] = obd.fuelLevel;
    obdObj["VIN"] = obd.VIN;
    obdObj["activeDTC"] = obd.activeDTC;
    obdObj["oilTemperature"] = obd.oilTemperature;
    obdObj["coolantTemperature"] = obd.coolantTemperature;
    obdObj["throttlePosition"] = obd.throttlePosition;
    obdObj["odometer"] = obd.odometer;

    JsonObject deviceObj = doc.createNestedObject("device");
    deviceObj["batteryInterval"] = device.batteryInterval;
    deviceObj["ignitionVoltageThreshold"] = device.ignitionVoltageThreshold;
    deviceObj["ignitionOffDelay"] = device.ignitionOffDelay;
    deviceObj["movementLogInterval"] = device.movementLogInterval;
    deviceObj["stopLogInterval"] = device.stopLogInterval;
    deviceObj["powerOutageDetection"] = device.powerOutageDetection;
    deviceObj["privateBusinessRide"] = device.privateBusinessRide;

    JsonObject accelerometerObj = doc.createNestedObject("accelerometer");
    accelerometerObj["movementThreshold"] = accelerometer.movementThreshold;
    accelerometerObj["movementStopDelay"] = accelerometer.movementStopDelay;
    accelerometerObj["logEnable"] = accelerometer.logEnable;
    accelerometerObj["crashDetectionThreshold"] = accelerometer.crashDetectionThreshold;
    accelerometerObj["towDetection"] = accelerometer.towDetection;

    JsonObject blackBoxObj = doc.createNestedObject("blackBox");
    blackBoxObj["interval"] = blackBox.interval;
    // Add black box parameters here if any
    blackBoxObj["parameter1"] = blackBox.parameter1;
    blackBoxObj["parameter2"] = blackBox.parameter2;

    JsonObject bluetoothObj = doc.createNestedObject("bluetooth");
    bluetoothObj["deviceManagementEnable"] = bluetooth.deviceManagementEnable;
    JsonArray devices = bluetoothObj.createNestedArray("devices");
    for (uint8_t i = 0; i < MAX_BLUETOOTH_DEVICES; ++i)
    {
        JsonObject device = devices.createNestedObject();
        device["type"] = static_cast<int>(bluetooth.devices[i].type);
        char bt[17];
        sprintf(bt, "%02x:%02x:%02x:%02x:%02x:%02x",
                bluetooth.devices[i].macAddress[0u],
                bluetooth.devices[i].macAddress[1u],
                bluetooth.devices[i].macAddress[2u],
                bluetooth.devices[i].macAddress[3u],
                bluetooth.devices[i].macAddress[4u],
                bluetooth.devices[i].macAddress[5u]);
        device["macAddress"] = bt;
    }

    Serial.print("Device configuration:\r\n");
    serializeJsonPretty(doc, Serial);
    Serial.print("\r\n");
}
