#pragma once

#include <Arduino.h>

struct ModemStatusData
{
    bool pinEnabled = false;
    String modemName;
    String modemInfo;
    String modemImei;
    String simCcid;
    String simImsi;
    String gsmOperator;
    String gsmNetworkType;
    String gsmFrequency;

    int16_t signal;
};