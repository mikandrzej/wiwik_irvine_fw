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
#include <Service.h>
#include <ModemManagement.h>
#include <DataLogger.h>
#include <TaskDataLogger.h>

#include <HwConfiguration.h>

#include <TaskCan.h>

#include <VehicleDataReporterTask.h>

#include <TaskVehicle.h>

#include <GpsObserver.h>

#include <nvs_flash.h>

const char MODULE[] = "MAIN";

#define TIMER0_INTERVAL_MS 1000

uint32_t software_version = 5u;

#define BLE_TASK_STACK_SIZE 16384
StaticTask_t xBleTaskBuffer;
StackType_t xBleStack[BLE_TASK_STACK_SIZE];
TaskHandle_t xBleTaskHandle = NULL;

#define DATA_LOGGER_TASK_STACK_SIZE 4096
StaticTask_t xDataLoggerTaskBuffer;
StackType_t xDataLoggerStack[DATA_LOGGER_TASK_STACK_SIZE];
TaskHandle_t xDataLoggerTaskHandle = NULL;

#define CAN_TASK_STACK_SIZE 4096
StaticTask_t xCanTaskBuffer;
StackType_t xCanStack[CAN_TASK_STACK_SIZE];
TaskHandle_t xCanTaskHandle = NULL;

#define DREP_TASK_STACK_SIZE 4096
StaticTask_t xDRepTaskBuffer;
StackType_t xDRepStack[DREP_TASK_STACK_SIZE];
TaskHandle_t xDrepTaskHandle = NULL;

#define VEHICLE_TASK_STACK_SIZE 4096
StaticTask_t xVehicleTaskBuffer;
StackType_t xVehicleStack[VEHICLE_TASK_STACK_SIZE];
TaskHandle_t xVehicleTaskHandle = NULL;

void setup()
{
  WiFi.mode(WIFI_OFF);

  SPI.begin(BOARD_SCK_PIN, BOARD_MISO_PIN, BOARD_MOSI_PIN);
  Serial.begin(CONSOLE_UART_BAUD);

  logger.begin(&Serial);
  dataLogger.begin();
  irvineConfiguration.begin();
  modemManagement.begin();
  service.begin();
  gpsObserver.begin();

  setCpuFrequencyMhz(240);
  
  logger.logPrintF(LogSeverity::INFO, MODULE, "CPU freq %d", getCpuFrequencyMhz());

  logger.logPrintF(LogSeverity::INFO, MODULE, "Application started");

  pinMode(BOARD_CAN_SE_PIN, OUTPUT);
  digitalWrite(BOARD_CAN_SE_PIN, LOW);

  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)BOARD_CAN_TX_PIN, (gpio_num_t)BOARD_CAN_RX_PIN, TWAI_MODE_NO_ACK);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

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

  // twai_message_t msg = {
  //     .flags = 0u,
  //     .identifier = 0x1234u,
  //     .data_length_code = 8,
  //     .data = {0},
  // };

  // msg.extd = 1;

  // while (1)
  // {
  //   msg.data[0]++;

  //   twai_transmit(&msg, 100);
  // }

  Serial.printf("Software version: %u\r\n", software_version);

  xBleTaskHandle = xTaskCreateStaticPinnedToCore(
      taskBluetooth,       /* Function that implements the task. */
      "BLE",               /* Text name for the task. */
      BLE_TASK_STACK_SIZE, /* Number of indexes in the xStack array. */
      (void *)1,           /* Parameter passed into the task. */
      tskIDLE_PRIORITY,    /* Priority at which the task is created. */
      xBleStack,           /* Array to use as the task's stack. */
      &xBleTaskBuffer,
      1); /* Variable to hold the task's data structure. */

  xDataLoggerTaskHandle = xTaskCreateStaticPinnedToCore(
      taskDataLogger,              /* Function that implements the task. */
      "D_LOGGER",                  /* Text name for the task. */
      DATA_LOGGER_TASK_STACK_SIZE, /* Number of indexes in the xStack array. */
      (void *)1,                   /* Parameter passed into the task. */
      tskIDLE_PRIORITY,            /* Priority at which the task is created. */
      xDataLoggerStack,            /* Array to use as the task's stack. */
      &xDataLoggerTaskBuffer,
      1); /* Variable to hold the task's data structure. */

  xCanTaskHandle = xTaskCreateStaticPinnedToCore(
      taskCan,             /* Function that implements the task. */
      "CAN",               /* Text name for the task. */
      CAN_TASK_STACK_SIZE, /* Number of indexes in the xStack array. */
      (void *)1,           /* Parameter passed into the task. */
      tskIDLE_PRIORITY,    /* Priority at which the task is created. */
      xCanStack,           /* Array to use as the task's stack. */
      &xCanTaskBuffer,
      1); /* Variable to hold the task's data structure. */

  xDrepTaskHandle = xTaskCreateStaticPinnedToCore(
      taskVehicleDataReporter, /* Function that implements the task. */
      "DREP",                  /* Text name for the task. */
      DREP_TASK_STACK_SIZE,    /* Number of indexes in the xStack array. */
      (void *)1,               /* Parameter passed into the task. */
      tskIDLE_PRIORITY,        /* Priority at which the task is created. */
      xDRepStack,              /* Array to use as the task's stack. */
      &xDRepTaskBuffer,
      1); /* Variable to hold the task's data structure. */

  xVehicleTaskHandle = xTaskCreateStaticPinnedToCore(
      taskVehicle,             /* Function that implements the task. */
      "VEH",                   /* Text name for the task. */
      VEHICLE_TASK_STACK_SIZE, /* Number of indexes in the xStack array. */
      (void *)1,               /* Parameter passed into the task. */
      tskIDLE_PRIORITY,        /* Priority at which the task is created. */
      xVehicleStack,           /* Array to use as the task's stack. */
      &xVehicleTaskBuffer,
      1); /* Variable to hold the task's data structure. */
}

void loop()
{
  modemManagement.loop();
  gpsObserver.loop();
  vTaskDelay(pdMS_TO_TICKS(1000));
}
