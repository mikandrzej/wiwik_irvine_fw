#pragma once

#include <Arduino.h>
#include <ArduinoBLE.h>
#include <vector>

constexpr int MAX_DISCOVERED_DEVICES = 50;
constexpr int DISCOVERY_TIMEOUT = 10000;

using bleTemperatureCallback = void(String, float &);

enum class BleTemperatureState
{
    NOT_INITIALISED,
    SCANNING_PREPARE,
    SCANNING,
    SCANNING_DELAY,
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
    String sensor_address = "";
    uint32_t m_period_timestamp;

    std::function<void(String &, float)> temperatiure_callback = nullptr;
    BleTemperatureState state = BleTemperatureState::NOT_INITIALISED;
};