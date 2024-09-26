#include "ConfigurationHelpers.h"
#include <stdio.h>

bool ConfigurationHelpers::copyString(const char *const src, char *const dst, const size_t dst_size)
{
    size_t len = strnlen(src, dst_size);
    if (len < sizeof(dst_size))
    {
        memcpy(dst, src, len);
        memset(&dst[len], 0, sizeof(dst_size) - len);
    }
    return false;
}

bool ConfigurationHelpers::copyMacAddress(const char *const src, uint8_t *const dst)
{
    int result = sscanf(src, "%hhx%hhx%hhx%hhx%hhx%hhx",
                        &dst[0], &dst[1], &dst[2],
                        &dst[3], &dst[4], &dst[5]);
    return result == 6;
}