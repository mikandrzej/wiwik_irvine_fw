#include "Vehicle.h"

#include <HwConfiguration.h>
#include <driver/adc.h>
#include "esp_adc_cal.h"
#include <IrvineConfiguration.h>
#include <Logger.h>
#include <CanManager.h>
#include <CanQueries/UdsVehicleSpeedQuery.h>
#include <CanQueries/UdsVehicleEngineSpeedQuery.h>

const char MODULE[] = "VEHICLE";
static esp_adc_cal_characteristics_t adc1_chars;

Vehicle vehicle;

void Vehicle::init()
{
    adcCalibration();
    adc_power_acquire();
    if (ESP_OK != adc1_config_width(ADC_WIDTH_BIT_12))
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "Failed to config ADC1 width");
    }
    if (ESP_OK != adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_12))
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "Failed to config ADC1 width");
    }
}

void Vehicle::loop()
{
    obtainVccVoltage();
}

float Vehicle::getVccVoltage(bool *valid)
{
    if (valid != nullptr)
    {
        if (voltage >= 0.0f)
            *valid = true;
        else
            *valid = false;
    }
    return voltage;
}

bool Vehicle::isIgnitionOn(bool *valid)
{
    switch (irvineConfiguration.vehicle.ignitionSource)
    {
    case VehicleIgnitionSource::CAN:
        return udsVehicleEngineSpeedQuery.getEngineSpeed(valid) > 200;
    case VehicleIgnitionSource::VOLTAGE:
        return isEngineRunningBasedOnVoltage(valid);
    case VehicleIgnitionSource::ALWAYS_ON:
        if (valid)
            *valid = true;
        return true;
    }

    if (valid)
        *valid = false;
    return false;
}

bool Vehicle::isEngineRunningBasedOnVoltage(bool *valid)
{
    if (valid != nullptr)
    {
        if (voltage >= 0.0f)
            *valid = true;
        else
            *valid = false;
    }

    if (voltage < 13.5f)
        return false;
    else if (voltage < 18.0f)
        return true;
    else if (voltage < 27.0f)
        return false;
    else
        return true;
}

bool Vehicle::isMoving(bool *valid)
{
    switch (irvineConfiguration.vehicle.movementDetectionSource)
    {
    case VehicleMovementDetectionSource::CAN:
        return udsVehicleSpeedQuery.getSpeed(valid) > 0;
    case VehicleMovementDetectionSource::ACCELEROMETER:
        if (valid)
            *valid = false;
        return 0;
    // case VehicleMovementDetectionSource::GPS:
    //     // gpsController.isMoving(valid);
    case VehicleMovementDetectionSource::VOLTAGE:
        return isEngineRunningBasedOnVoltage(valid);
    }
    if (valid)
        *valid = false;
    return false;
}

float Vehicle::getSpeed(bool *valid)
{
    switch (irvineConfiguration.vehicle.speedSource)
    {
    case VehicleSpeedSource::CAN:
        return udsVehicleSpeedQuery.getSpeed(valid);
    // case VehicleSpeedSource::GPS:
    //     return gpsController.getGpsData(valid).speed;
    }

    if (valid)
        *valid = false;
    return 0.0f;
}

void Vehicle::obtainVccVoltage()
{
    uint32_t rawVoltage = (uint32_t)adc1_get_raw(ADC1_CHANNEL_7);
    if (calibrationEnabled)
    {
        rawVoltage = esp_adc_cal_raw_to_voltage(rawVoltage, &adc1_chars);
    }

    voltage = (float)(rawVoltage * 11u) / 1000.0f;
    logger.logPrintF(LogSeverity::DEBUG, MODULE, "VCC voltage read: %.3f", voltage);
}

void Vehicle::adcCalibration()
{
    esp_err_t ret;

    ret = esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF);
    if (ret == ESP_ERR_NOT_SUPPORTED)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "Calibration scheme not supported, skip software calibration");
    }
    else if (ret == ESP_ERR_INVALID_VERSION)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "eFuse not burnt, skip software calibration");
    }
    else if (ret == ESP_OK)
    {
        calibrationEnabled = true;
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 0, &adc1_chars);
    }
    else
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "Invalid arg");
    }
}
