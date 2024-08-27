#include "Vehicle.h"

#include <HwConfiguration.h>
#include <driver/adc.h>
#include "esp_adc_cal.h"
#include <Logger.h>

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
    if (ESP_OK != adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11))
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
    if (voltage >= 0.0f)
        *valid = true;
    else
        *valid = false;
    return voltage;
}

void Vehicle::obtainVccVoltage()
{
    uint32_t rawVoltage = (uint32_t)adc1_get_raw(ADC1_CHANNEL_7);
    ;
    if (calibrationEnabled)
    {
        rawVoltage = esp_adc_cal_raw_to_voltage(rawVoltage, &adc1_chars);
    }

    voltage = (float)(rawVoltage * 11u);
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
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 0, &adc1_chars);
    }
    else
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "Invalid arg");
    }
}