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

const char MODULE[] = "TASK_BT";
static esp_ble_scan_params_t ble_scan_params = {
    .scan_type = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval = 0x1000,
    .scan_window = 0x950,
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE};

class TaskBluetooth
{
public:
    void loop();
};

static void esp_gatts_callback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void esp_gap_callback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

void taskBluetooth(void *pvParameters)
{
    TaskBluetooth taskData;

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
        taskData.loop();
        vTaskDelay(1);
    }
}

static void esp_gatts_callback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    ESP_LOGI(MODULE, "esp_gatts_callback");
    if (event == ESP_GATTS_REG_EVT)
    {
        ESP_LOGE(MODULE, "reg app failed, app_id %04x, status %d",
                 param->reg.app_id,
                 param->reg.status);
        return;
    }
}
static void print_manufacturer_data(uint8_t *adv_data, uint8_t adv_data_len)
{
    uint8_t index = 0;
    while (index < adv_data_len)
    {
        uint8_t length = adv_data[index++];
        if (length == 0)
        {
            break;
        }

        uint8_t type = adv_data[index];
        if (type == 0xFF)
        {
            char manufacturer_data_str[256] = {0};
            int offset = 0;
            for (int i = 0; i < length - 1; i++)
            {
                offset += snprintf(manufacturer_data_str + offset, sizeof(manufacturer_data_str) - offset, "%02x ", adv_data[index + 1 + i]);
            }
            logger.logPrintF(LogSeverity::INFO, MODULE, "manufacturer data: %s", manufacturer_data_str);
        }

        index += length;
    }
}
static void print_adv_data(uint8_t *adv_data, uint8_t adv_data_len)
{
    char data_str[256] = {0};
    int offset = 0;
    for (int i = 0; i < adv_data_len; i++)
    {
        offset += snprintf(data_str + offset, sizeof(data_str) - offset, "%02x", adv_data[i]);
    }
    logger.logPrintF(LogSeverity::INFO, MODULE, "adv data: %s", data_str);
}

static void esp_gap_callback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    esp_err_t err;

    ESP_LOGI(MODULE, "esp_gap_callback");
    switch (event)
    {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
    {
        uint32_t duration = 0; // 0 indicates continuous scanning
        esp_ble_gap_start_scanning(duration);
        ESP_LOGI(MODULE, "ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT");
        break;
    }
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
    {
        if ((err = param->scan_start_cmpl.status) != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(MODULE, "Scan start failed: %s", esp_err_to_name(err));
        }
        else
        {
            ESP_LOGI(MODULE, "Start scanning...");
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
                    logger.logPrintF(LogSeverity::INFO, MODULE, "scan result: %02x:%02x:%02x:%02x:%02x:%02x rssi %d",
                                     scan_result->scan_rst.bda[0],
                                     scan_result->scan_rst.bda[1],
                                     scan_result->scan_rst.bda[2],
                                     scan_result->scan_rst.bda[3],
                                     scan_result->scan_rst.bda[4],
                                     scan_result->scan_rst.bda[5],
                                     scan_result->scan_rst.rssi);
                    print_adv_data(scan_result->scan_rst.ble_adv, scan_result->scan_rst.adv_data_len);
                    print_manufacturer_data(scan_result->scan_rst.ble_adv, scan_result->scan_rst.adv_data_len);
                }
            }

            // Print advertised data

            break;
        }
        default:
            break;
        }
        break;
    }
    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
    {
        if ((err = param->scan_stop_cmpl.status) != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(MODULE, "Scan stop failed: %s", esp_err_to_name(err));
        }
        else
        {
            ESP_LOGI(MODULE, "Stop scan successfully");
        }
        break;
    }
    default:
        break;
    }
}

void TaskBluetooth::loop()
{
}
