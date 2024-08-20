#include "TaskCan.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "CanManager.h"

const char MODULE[] = "TASK_CAN";

void taskCan(void *)
{
    canManager.setup();

    while (1)
    {
        canManager.loop();
        vTaskDelay(1);
    }
}