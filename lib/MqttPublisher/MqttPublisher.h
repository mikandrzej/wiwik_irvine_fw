#include <stdint.h>

enum class MqttDataType
{
    SD_STATUS,
    MODEM_STATUS,
    VEHICLE_DATA,
    MEASURE
};

class MqttPublisher
{
public:
    static void publishSdData(uint64_t used_mb, uint64_t total_mb, uint64_t card_size);
};