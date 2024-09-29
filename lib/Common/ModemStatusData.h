#pragma once

#include <Arduino.h>

struct ModemStatusData
{
    bool pinEnabled = false;
    bool simReady = false;
    bool modemPoweredOn = false;
    bool gprsConnected = false;
    bool mqttConnected = false;
    bool gpsEnabled = false;
    String modemName="";
    String modemModel="";
    String modemImei="";
    String simCcid="";
    String simImsi="";
    String gsmOperator="";
    String gsmNetworkType="";
    String gsmFrequency="";

    int16_t signal = 99;
};