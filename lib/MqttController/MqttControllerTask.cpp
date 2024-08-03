#include "MqttControllerTask.h"

#include <MqttController.h>

void mqttControllerTask(void *)
{
    mqttController.begin();

    while (1)
    {
        mqttController.loop();
        vTaskDelay(1);
    }
}