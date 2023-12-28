
#include "Configuration.h"
#include "Arduino.h"
#include "Debug.h"
#include "Comm.h"

Debug debug;

void Debug::debug(const String &str)
{
    if (configuration.getDebugMode() >= DEBUG_LOG)
        comm.publish_debug_data("DEBUG: " + str);
    Serial.print("DEBUG: " + str);
}

void Debug::warning(const String &str)
{
    if (configuration.getDebugMode() >= DEBUG_WARNING)
        comm.publish_debug_data("WARNING: " + str);
    Serial.print("WARNING: " + str);
}

void Debug::error(const String &str)
{
    if (configuration.getDebugMode() >= DEBUG_ERROR)
        comm.publish_debug_data("ERROR: " + str);
    Serial.print("ERROR: " + str);
}