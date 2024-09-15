#include "TaskBluetooth.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <Logger.h>
#include <IrvineConfiguration.h>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gatts_api.h"

#include "AbstractBluetoothBleDevice.h"
#include "BluetoothJaaleeTempSensor.h"
#include "BluetoothM52PASTempSensor.h"

const char MODULE[] = "TASK_BT";
static esp_ble_scan_params_t ble_scan_params = {
    .scan_type = BLE_SCAN_TYPE_PASSIVE,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval = 0x4000,
    .scan_window = 0x1000,
    .scan_duplicate = BLE_SCAN_DUPLICATE_ENABLE};

class TaskBluetooth
{
public:
    void setup();
    void loop();
    void parseAdvertisedData(const uint16_t deviceIndex, const uint8_t *const data, const uint16_t len, int16_t rssi);
    void logDiscoveredDevice(const uint8_t *const mac, const uint8_t *const data, const uint8_t len, int rssi);

private:
    std::vector<AbstractBluetoothBleDevice *> devices;
};

static void esp_gatts_callback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void esp_gap_callback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

TaskBluetooth bluetoothTaskData;

void taskBluetooth(void *pvParameters)
{
    bluetoothTaskData.setup();
    // todo move below to setup

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    bt_cfg.mode = ESP_BT_MODE_BLE;
    esp_err_t esp_err = esp_bt_controller_init(&bt_cfg);
    if (esp_err != ESP_OK)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "esp_bt_controller_init error 0x%x", esp_err);
        vTaskDelete(NULL);
    }
    esp_err = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (esp_err != ESP_OK)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "esp_bt_controller_enable error 0x%x", esp_err);
        vTaskDelete(NULL);
    }
    esp_err = esp_bluedroid_init();
    if (esp_err != ESP_OK)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "esp_bluedroid_init error 0x%x", esp_err);
        vTaskDelete(NULL);
    }
    esp_err = esp_bluedroid_enable();
    if (esp_err != ESP_OK)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "esp_bluedroid_enable error 0x%x", esp_err);
        vTaskDelete(NULL);
    }
    esp_err = esp_ble_gatts_register_callback(esp_gatts_callback);
    if (esp_err != ESP_OK)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "esp_ble_gap_register_callback error 0x%x", esp_err);
        vTaskDelete(NULL);
    }
    esp_err = esp_ble_gap_register_callback(esp_gap_callback);
    if (esp_err != ESP_OK)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "esp_ble_gap_register_callback error 0x%x", esp_err);
        vTaskDelete(NULL);
    }

    esp_ble_gap_set_scan_params(&ble_scan_params);

    while (1)
    {
        bluetoothTaskData.loop();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void esp_gatts_callback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    if (event == ESP_GATTS_REG_EVT)
    {
        logger.logPrintF(LogSeverity::ERROR, MODULE, "reg app failed, app_id %04x, status %d",
                         param->reg.app_id,
                         param->reg.status);
        return;
    }
}

static void esp_gap_callback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    esp_err_t err;

    switch (event)
    {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
    {
        uint32_t duration = 0; // 0 indicates continuous scanning
        esp_ble_gap_start_scanning(duration);
        logger.logPrintF(LogSeverity::DEBUG, MODULE, "ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT");
        break;
    }
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
    {
        if ((err = param->scan_start_cmpl.status) != ESP_BT_STATUS_SUCCESS)
        {
            logger.logPrintF(LogSeverity::ERROR, MODULE, "Scan start failed: %s", esp_err_to_name(err));
        }
        else
        {
            logger.logPrintF(LogSeverity::INFO, MODULE, "Start scanning...");
        }
        break;
    }
    case ESP_GAP_BLE_SCAN_RESULT_EVT:
    {
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
        switch (scan_result->scan_rst.search_evt)
        {
        case ESP_GAP_SEARCH_INQ_RES_EVT:
        {
            for (int k = 0; k < MAX_BLUETOOTH_DEVICES; k++)
            {
                if (!memcmp(irvineConfiguration.bluetooth.devices[k].macAddress, scan_result->scan_rst.bda, 6u))
                {
                    bluetoothTaskData.parseAdvertisedData(k, scan_result->scan_rst.ble_adv, scan_result->scan_rst.adv_data_len, scan_result->scan_rst.rssi);
                }
            }
            bluetoothTaskData.logDiscoveredDevice((uint8_t *)scan_result->scan_rst.bda, scan_result->scan_rst.ble_adv, scan_result->scan_rst.adv_data_len, scan_result->scan_rst.rssi);

            break;
        }
        default:
            logger.logPrintF(LogSeverity::INFO, MODULE, "ble scan result: %d", scan_result->scan_rst.search_evt);
            break;
        }
        break;
    }
    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
    {
        if ((err = param->scan_stop_cmpl.status) != ESP_BT_STATUS_SUCCESS)
        {
            logger.logPrintF(LogSeverity::ERROR, MODULE, "Scan stop failed: %s", esp_err_to_name(err));
        }
        else
        {
            logger.logPrintF(LogSeverity::INFO, MODULE, "Stop scan successfully");
        }
        break;
    }
    default:
        break;
    }
}

void TaskBluetooth::setup()
{
    for (uint16_t dev = 0u; dev < MAX_BLUETOOTH_DEVICES; dev++)
    {
        switch (irvineConfiguration.bluetooth.devices[dev].type)
        {
        case BluetoothDeviceType::JAALEE_SENSOR:
            // todo shared pointer
            devices.push_back(new BluetoothJaaleeTempSensor(dev));
            break;
        case BluetoothDeviceType::M52PAS_SENSOR:
            // todo shared pointer
            devices.push_back(new BluetoothM52PASTempSensor(dev));
            break;
        default:
            devices.push_back(nullptr);
            break;
        }
    }
}

void TaskBluetooth::loop()
{
}

void TaskBluetooth::parseAdvertisedData(const uint16_t deviceIndex, const uint8_t *const data, const uint16_t len, int16_t rssi)
{
    devices[deviceIndex]->parseAdvertisedData(data, len, rssi);
}

void TaskBluetooth::logDiscoveredDevice(const uint8_t *const mac, const uint8_t *const data, const uint8_t len, int rssi)
{
    return;

    char txt[500];
    int txt_len = sprintf(txt, "BLE device (RSSI=%d): ", rssi);
    for (uint16_t k = 0u; k < 6u; k++)
    {
        if (k > 0u)
            txt[txt_len++] = ':';
        txt_len += sprintf(&txt[txt_len], "%02X", mac[k]);
    }
    txt_len += sprintf(&txt[txt_len], " data(%u):", len);

    for (uint16_t k = 0u; k < len; k++)
    {
        txt_len += sprintf(&txt[txt_len], "%02X", data[k]);
    }

    logger.logPrintF(LogSeverity::DEBUG, MODULE, txt);
}