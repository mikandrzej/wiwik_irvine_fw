#include "BLEJaaleTemperature.h"
#include "Configuration.h"

void BleJaaleeTemperature::loop()
{
    switch (this->state)
    {
    case BleTemperatureState::NOT_INITIALISED:
        if (BLE.begin())
        {
            this->state = BleTemperatureState::SCANNING_PREPARE;
            Serial.printf("Going to SCANNING_PREPARE state\r\n");
        }
        else
        {
            Serial.printf("BLE init error\r\n");
        }
        break;

    case BleTemperatureState::SCANNING_PREPARE:
        BLE.scanForAddress(this->ble_address);
        m_period_timestamp = millis();
        this->state = BleTemperatureState::SCANNING;
        Serial.printf("Going to SCANNING state\r\n");
        break;

    case BleTemperatureState::SCANNING:
    {
        BLEDevice peripheral = BLE.available();
        if (peripheral)
        {
            uint8_t man_data[128];
            int man_data_len = peripheral.manufacturerData(man_data, 128);
            if (man_data_len == 26)
            {
                this->parseValue(&man_data[20u], 2);
            }
            BLE.stopScan();
            this->state = BleTemperatureState::SCANNING_DELAY;
            Serial.printf("Going to SCANNING_DELAY state\r\n");
        }
        break;
    }

    case BleTemperatureState::SCANNING_DELAY:
    {

        uint32_t t = millis();
        uint32_t diff = t - this->m_period_timestamp;
        if (diff > configuration.getTemperatureReportInterval())
        {
            this->state = BleTemperatureState::SCANNING_PREPARE;
            Serial.printf("Going to SCANNING_PREPARE state\r\n");
        }

        break;
    }
    }
}

void BleJaaleeTemperature::setAddress(String ble_address)
{
    this->ble_address = ble_address;
    ble_address.replace(":", "");
    this->sensor_address = String("jaale_0x") + ble_address;
}

void BleJaaleeTemperature::setTemperatureCallback(const std::function<void(String &, float)> &newTemperatureCallback)
{
    this->temperatiure_callback = newTemperatureCallback;
}

bool BleJaaleeTemperature::parseValue(const uint8_t *raw, uint8_t len)
{
    uint16_t parsed = 0u;
    for (int i = 0; i < 2; i++)
    {
        parsed <<= 8;
        parsed |= raw[i];
    }
    if (parsed > 0u)
    {
        double temperature = 175.0 * ((double)parsed / 65535.0) - 45.0;
        Serial.printf("BLE Jaale Temperature received: %.2f\r\n", temperature);
        if (this->temperatiure_callback)
        {
            this->temperatiure_callback(this->sensor_address, temperature);
        }
        return true;
    }
    return false;
}
