#include "TaskGps.h"

#include "GpsController.h"

void taskGps(void *)
{
    while (1)
    {
        gpsController.loop();

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}