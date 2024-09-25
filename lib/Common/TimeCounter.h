#define TIME_COUNT_BEGIN \
{ \
uint32_t _start_time = millis();

#define TIME_COUNT_END(x) \
uint32_t _diff = millis() - _start_time; \
logger.logPrintF(LogSeverity::DEBUG, MODULE, x " takes %u ms", _diff); \
}
