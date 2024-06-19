#include <Arduino.h>

#include <WiFi.h>
#include <SPI.h>

#include "Irvine.h"
#include "Configuration.h"
#include "ESP32TimerInterrupt.h"
#include "BLETask.h"

#include <Update.h>
#define TIMER0_INTERVAL_MS 1000

#define BOARD_MISO_PIN (2)
#define BOARD_MOSI_PIN (15)
#define BOARD_SCK_PIN (14)

Irvine irvine;

uint32_t software_version = 5u;

/* The variable used to hold the queue's data structure. */
static StaticQueue_t xStaticQueue;

/* The array to use as the queue's storage area.  This must be at least
uxQueueLength * uxItemSize bytes. */
uint8_t ucQueueStorageArea[BLE_QUEUE_LENGTH * BLE_QUEUE_ITEM_SIZE];

#define BLE_TASK_STACK_SIZE 5000
StaticTask_t xBleTaskBuffer;
StackType_t xBleStack[BLE_TASK_STACK_SIZE];
TaskHandle_t xBleTaskHandle = NULL;

void setup()
{
  SPI.begin(BOARD_SCK_PIN, BOARD_MISO_PIN, BOARD_MOSI_PIN);
  Serial.begin(115200);

  pinMode(21, OUTPUT); // 1wire
  pinMode(22, OUTPUT);

  xBleQueue = xQueueCreateStatic(BLE_QUEUE_LENGTH,
                                 BLE_QUEUE_ITEM_SIZE,
                                 ucQueueStorageArea,
                                 &xStaticQueue);

  configuration.initSource();
  configuration.readConfig();

  Serial.printf("Software version: %u\r\n", software_version);

  /* Create the task without using any dynamic memory allocation. */
  xBleTaskHandle = xTaskCreateStatic(
      BLETask,             /* Function that implements the task. */
      "BLE",               /* Text name for the task. */
      BLE_TASK_STACK_SIZE, /* Number of indexes in the xStack array. */
      (void *)1,           /* Parameter passed into the task. */
      tskIDLE_PRIORITY,    /* Priority at which the task is created. */
      xBleStack,           /* Array to use as the task's stack. */
      &xBleTaskBuffer);    /* Variable to hold the task's data structure. */
}

void loop()
{
  irvine.loop();
  configuration.cyclic();
}
