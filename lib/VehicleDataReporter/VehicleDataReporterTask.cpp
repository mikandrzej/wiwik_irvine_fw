#include "VehicleDataReporterTask.h"

#include "VehicleDataReporter.h"

void taskVehicleDataReporter(void *)
{
    vehicleDataReporter.init();
    while (1)
    {
        vehicleDataReporter.loop();

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}