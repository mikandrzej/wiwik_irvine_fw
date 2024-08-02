#pragma once

#include <stdint.h>
#include <Preferences.h>

#include "ServerConfiguration.h"
#include "ModemConfiguration.h"
#include "SmsConfiguration.h"
#include "TimeConfiguration.h"
#include "VehicleConfiguration.h"
#include "GpsConfiguration.h"
#include "ObdConfiguration.h"
#include "DeviceConfiguration.h"
#include "AccelerometerConfiguration.h"
#include "BlackBoxConfiguration.h"
#include "BluetoothConfiguration.h"

class IrvineConfiguration
{
public:
    bool begin();
    void printConfiguration();

    ServerConfiguration server;
    ModemConfiguration modem;
    SmsConfiguration sms;
    TimeConfiguration time;
    VehicleConfiguration vehicle;
    GpsConfiguration gps;
    ObdConfiguration obd;
    DeviceConfiguration device;
    AccelerometerConfiguration accelerometer;
    BlackBoxConfiguration blackBox;
    BluetoothConfiguration bluetooth;

private:
    void loadServerConfig();
    void loadModemConfig();
    void loadSmsConfig();
    void loadTimeConfig();
    void loadVehicleConfig();
    void loadGpsConfig();
    void loadObdConfig();
    void loadDeviceConfig();
    void loadAccelerometerConfig();
    void loadBlackBoxConfig();
    void loadBluetoothConfig();

    Preferences preferences;
};

extern IrvineConfiguration irvineConfiguration;
