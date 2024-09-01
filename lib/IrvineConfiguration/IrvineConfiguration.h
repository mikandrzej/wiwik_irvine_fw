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

enum class ConfigurationParameterType
{
    BYTES,
    STR,
    UCHAR,
    USHORT,
    UINT,
    BOOL
};

struct ConfigurationParameter
{
    const char *const name;
    ConfigurationParameterType type;
    void *pointer;
    uint32_t len;
};

class IrvineConfiguration
{
public:
    bool begin();
    void printConfiguration();

    bool setParameter(const char *param, const char *value);

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
    void checkDefaultConfiguration(void);
    void printParameter(ConfigurationParameter &parameter);
    void loadParameter(ConfigurationParameter &parameter);

    Preferences preferences;
};

extern IrvineConfiguration irvineConfiguration;
