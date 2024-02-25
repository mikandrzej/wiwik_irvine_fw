#include "BLEJaaleTemperature.h"
#include "Configuration.h"

void BleJaaleeTemperature::loop()
{
    switch (this->state)
    {
    case BleTemperatureState::NOT_INITIALISED:
        if (BLE.begin())
        {
            this->state = BleTemperatureState::NOT_CONNECTED;
            Serial.printf("Going to NOT_CONNECTED state\r\n");
        }
        else
        {
            Serial.printf("BLE init error\r\n");
        }
        break;
    case BleTemperatureState::NOT_CONNECTED:
        BLE.scanForAddress(this->ble_address);
        this->state = BleTemperatureState::SCANNING;
        Serial.printf("Going to SCANNING state\r\n");
        break;
    case BleTemperatureState::SCANNING:
        this->device = BLE.available();
        if (this->device)
        {
            BLE.stopScan();
            this->state = BleTemperatureState::CONNECTING;
            Serial.printf("Going to CONNECTING state\r\n");
        }
        break;
    case BleTemperatureState::CONNECTING:
        if (this->device.connect())
        {
            this->state = BleTemperatureState::ATTRIBUTE_SCAN;
            Serial.printf("Going to ATTRIBUTE_SCAN state\r\n");
        }
        break;
    case BleTemperatureState::ATTRIBUTE_SCAN:
        if (!this->device.connected())
        {
            this->state = BleTemperatureState::NOT_CONNECTED;
            Serial.printf("Going to NOT_CONNECTED state\r\n");
        }
        else if (!this->device.discoverAttributes())
        {
            this->state = BleTemperatureState::NOT_CONNECTED;
            Serial.printf("Going to NOT_CONNECTED state\r\n");
        }
        else
        {
            this->state = BleTemperatureState::SERVICE_SCAN;
            Serial.printf("Going to SERVICE_SCAN state\r\n");
        }

    case BleTemperatureState::SERVICE_SCAN:
        if (!this->device.connected())
        {
            this->state = BleTemperatureState::NOT_CONNECTED;
            Serial.printf("Going to NOT_CONNECTED state\r\n");
        }
        else if (!this->device.hasService(this->service_uuid.c_str()))
        {
            this->state = BleTemperatureState::ATTRIBUTE_SCAN;
            Serial.printf("Going to ATTRIBUTE_SCAN state\r\n");
        }
        else
        {
            this->service = this->device.service(this->service_uuid.c_str());
            this->state = BleTemperatureState::CHARACTERISTIC_SCAN;
            Serial.printf("Going to CHARACTERISTIC_SCAN state\r\n");
        }
        break;
    case BleTemperatureState::CHARACTERISTIC_SCAN:
        if (!this->device.connected())
        {
            this->state = BleTemperatureState::NOT_CONNECTED;
            Serial.printf("Going to NOT_CONNECTED state\r\n");
        }
        else if (!this->service.hasCharacteristic(this->characteristic_uuid.c_str()))
        {
            this->state = BleTemperatureState::NOT_CONNECTED;
            Serial.printf("Going to NOT_CONNECTED state\r\n");
        }
        else
        {
            this->characteristic = this->service.characteristic(this->characteristic_uuid.c_str());
            this->state = BleTemperatureState::CONNECTED;
            Serial.printf("Going to CONNECTED state\r\n");
        }
        break;
    case BleTemperatureState::CONNECTED:
        if (!this->device.connected())
        {
            this->state = BleTemperatureState::NOT_CONNECTED;
            Serial.printf("Going to NOT_CONNECTED state\r\n");
        }
        else
        {
            uint32_t t = millis();
            uint32_t diff = t - this->m_period_timestamp;
            if (diff > configuration.getTemperatureReportInterval())
            {
                if (this->characteristic.read() && this->characteristic.valueLength() > 0)
                {
                    if (this->parseValue(this->characteristic.value(), this->characteristic.valueLength()))
                    {
                        m_period_timestamp = millis();
                    }
                }
            }
        }
        break;
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
