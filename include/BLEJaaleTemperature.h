#pragma once

#include <Arduino.h>
#include <ArduinoBLE.h>

using bleTemperatureCallback = void(String, float &);

enum class BleTemperatureState
{
    NOT_INITIALISED,
    NOT_CONNECTED,
    SCANNING,
    CONNECTING,
    ATTRIBUTE_SCAN,
    SERVICE_SCAN,
    CHARACTERISTIC_SCAN,
    CONNECTED
};

class BleJaaleeTemperature
{
public:
    void loop();
    void setAddress(String ble_address);
    void setTemperatureCallback(const std::function<void(String&, float)> &newTemperatureCallback);

private:
    bool parseValue(const uint8_t *raw, uint8_t len);

    String ble_address = "";
    const String device_name = "JAALEE";
    const String service_uuid = "ffb0";
    const String characteristic_uuid = "ffb1";
    const String descriptor_uuid = "";
    String sensor_address = "";
    uint32_t m_period_timestamp;

    BLEDevice device;
    BLEService service;
    BLECharacteristic characteristic;

    std::function<void(String &, float)> temperatiure_callback = nullptr;
    BleTemperatureState state = BleTemperatureState::NOT_INITIALISED;
};