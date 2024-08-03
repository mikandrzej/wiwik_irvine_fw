#include "TaskGps.h"

#include <Gps.h>

void taskGps(void *)
{
    while (1)
    {
        gps.loop();
        // todo vtaskdelayuntil
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}