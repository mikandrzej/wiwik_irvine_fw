#include <Arduino.h>

#include <WiFi.h>
#include <SPI.h>

#include <driver/twai.h>

#include "EgTinyGsm.h"
#include "ESP32TimerInterrupt.h"

#include <Update.h>

#include <IrvineConfiguration.h>
#include <Logger.h>

#include <TaskBluetooth.h>
#include <MqttControllerTask.h>
#include <Service.h>
#include <ModemManagement.h>

#include <TaskGps.h>

#define TIMER0_INTERVAL_MS 1000

#define BOARD_MISO_PIN (15)
#define BOARD_MOSI_PIN (13)
#define BOARD_SCK_PIN (2)
#define BOARD_CS_PIN (32)

#define BOARD_CAN_TX_PIN (33)
#define BOARD_CAN_RX_PIN (25)
#define BOARD_CAN_SE_PIN (4)

#define CONSOLE_UART_BAUD 115200

uint32_t software_version = 5u;

#define BLE_TASK_STACK_SIZE 4096
StaticTask_t xBleTaskBuffer;
StackType_t xBleStack[BLE_TASK_STACK_SIZE];
TaskHandle_t xBleTaskHandle = NULL;

#define MQTT_TASK_STACK_SIZE 4096
StaticTask_t xMqttTaskBuffer;
StackType_t xMqttStack[MQTT_TASK_STACK_SIZE];
TaskHandle_t xMqttTaskHandle = NULL;

#define GPS_TASK_STACK_SIZE 4096
StaticTask_t xGpsTaskBuffer;
StackType_t xGpsStack[GPS_TASK_STACK_SIZE];
TaskHandle_t xGpsTaskHandle = NULL;

void setup()
{
  SPI.begin(BOARD_SCK_PIN, BOARD_MISO_PIN, BOARD_MOSI_PIN);
  Serial.begin(CONSOLE_UART_BAUD);

  logger.begin(&Serial);
  irvineConfiguration.begin();
  modemManagement.begin();

  logger.logPrintF(LogSeverity::INFO, "MAIN", "Application started");

  pinMode(BOARD_CAN_SE_PIN, OUTPUT);
  digitalWrite(BOARD_CAN_SE_PIN, LOW);

  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)BOARD_CAN_TX_PIN, (gpio_num_t)BOARD_CAN_RX_PIN, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = {.acceptance_code = 0x00000000, .acceptance_mask = 0xFFFFFFFF, .single_filter = true};

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK)
  {
    printf("Failed to install TWAI driver\n");
    return;
  }
  if (twai_start() != ESP_OK)
  {
    printf("Failed to start TWAI driver\n");
    return;
  }

  Serial.printf("Software version: %u\r\n", software_version);

  // service.begin();

  xBleTaskHandle = xTaskCreateStaticPinnedToCore(
      mqttControllerTask,   /* Function that implements the task. */
      "MQTT",               /* Text name for the task. */
      MQTT_TASK_STACK_SIZE, /* Number of indexes in the xStack array. */
      (void *)1,            /* Parameter passed into the task. */
      tskIDLE_PRIORITY,     /* Priority at which the task is created. */
      xMqttStack,           /* Array to use as the task's stack. */
      &xMqttTaskBuffer,
      1); /* Variable to hold the task's data structure. */

  xMqttTaskHandle = xTaskCreateStaticPinnedToCore(
      taskBluetooth,       /* Function that implements the task. */
      "BLE",               /* Text name for the task. */
      BLE_TASK_STACK_SIZE, /* Number of indexes in the xStack array. */
      (void *)1,           /* Parameter passed into the task. */
      tskIDLE_PRIORITY,    /* Priority at which the task is created. */
      xBleStack,           /* Array to use as the task's stack. */
      &xBleTaskBuffer,
      1); /* Variable to hold the task's data structure. */

  xMqttTaskHandle = xTaskCreateStaticPinnedToCore(
      taskGps,             /* Function that implements the task. */
      "GPS",               /* Text name for the task. */
      GPS_TASK_STACK_SIZE, /* Number of indexes in the xStack array. */
      (void *)1,           /* Parameter passed into the task. */
      tskIDLE_PRIORITY,    /* Priority at which the task is created. */
      xGpsStack,           /* Array to use as the task's stack. */
      &xGpsTaskBuffer,
      1); /* Variable to hold the task's data structure. */
}

void loop()
{
  modemManagement.loop();
  vTaskDelay(1);
}
