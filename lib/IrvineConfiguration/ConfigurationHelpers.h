#include <string.h>

class ConfigurationHelpers
{
public:
    static bool copyString(const char *const src, char *const dst, const size_t dst_size);
    static bool copyMacAddress(const char *const src, uint8_t *const dst);
};