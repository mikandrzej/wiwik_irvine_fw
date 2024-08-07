#include "Irvine.h"
#include "Configuration.h"
#include "BLETask.h"

#include <Wire.h>
#include <memory>
#include <UdsFuelTankLevelQuery.hpp>

void Irvine::loop()
{
    ble_que_item_s ble_msg;

    if (initSm())
    {
        comm.loop();

        m_battery.loop();

        m_gps.loop();

        m_udsQueryManager.loop(millis());

        if (xQueueReceive(xBleQueue, &ble_msg, 0))
        {
            if (ble_msg.type == BLE_QUE_TYPE_TEMPERATURE)
            {
                String sensor_address = String(ble_msg.temperature.sensor_address);
                this->onTemperatureReady(sensor_address, ble_msg.temperature.value);
            }
        }
    }
}

boolean Irvine::initSm()
{

    switch (m_initState)
    {
    case INIT_STATE_BATTERY_INIT:
        if (batteryInit())
        {
            m_initState = INIT_STATE_GPS_INIT;
        }
        break;
    case INIT_STATE_GPS_INIT:
        if (gpsInit())
        {
            m_initState = INIT_STATE_UDS_INIT;
        }
        break;
    case INIT_STATE_UDS_INIT:
        if (udsInit())
        {
            m_initState = INIT_STATE_DONE;
        }
        break;
    case INIT_STATE_DONE:
        return true;
    }

    return false;
}

boolean Irvine::batteryInit()
{
    m_battery.setOnBatteryVoltageReady(
        [this](float voltage)
        {
            this->onBatteryVoltageReady(voltage);
        });

    comm.setBattery(&m_battery);

    return true;
}

boolean Irvine::gpsInit()
{
    m_gps.setOnGpsDataReady(
        [this](GpsData &gpsData)
        {
            this->onGpsDataReady(gpsData);
        });
    return true;
}

boolean Irvine::udsInit()
{
    auto tankLevelQuery = std::make_unique<UdsFuelTankLevelQuery>([this](const float value)
                                                                  { this->onTankLevel(value); }, 10000);

    m_udsQueryManager.addQuery(std::move(tankLevelQuery));
    return true;
}

void Irvine::onTemperatureReady(String &sensorAddress, float temperature)
{
    Serial.printf("temperature ready for %s: %.2f\n", sensorAddress.c_str(), temperature);
    comm.publish_measure_data("temperature", sensorAddress, temperature);
}

void Irvine::onBatteryVoltageReady(float voltage)
{
    String sensor_id = comm.getDeviceId() + "_battery";
    comm.publish_measure_data("battery", sensor_id, voltage);
}

void Irvine::onGpsDataReady(GpsData &gpsData)
{
    String sensor_id = comm.getDeviceId() + "_gps";
    DynamicJsonDocument doc(200);
    doc["tim"] = gpsData.m_timestamp;
    doc["lng"] = gpsData.m_longitude;
    doc["lat"] = gpsData.m_latitude;
    doc["alt"] = gpsData.m_altitude;
    doc["acc"] = gpsData.m_precision;
    doc["sat"] = gpsData.m_satellites;
    doc["spd"] = gpsData.m_speed;
    char buf[200];
    serializeJson(doc, buf, sizeof(buf));
    Serial.println(buf);
    comm.publish_measure_data("gps", sensor_id, doc);
}

void Irvine::onTankLevel(const float value)
{
    String sensor_id = comm.getDeviceId() + "_fuel";
    comm.publish_measure_data("fuel", sensor_id, value);
}
