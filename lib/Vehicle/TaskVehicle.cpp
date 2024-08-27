#include "TaskVehicle.h"

#include "Vehicle.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void taskVehicle(void *)
{
    vehicle.init();

    while (1)
    {
        vehicle.loop();

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}