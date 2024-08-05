#ifndef CTRONOME_LOGGING_ADAPTER_H
#define CTRONOME_LOGGING_ADAPTER_H
#define LEVEL_FATAL 0
#define LEVEL_ERROR 1
#define LEVEL_WARNING 2
#define LEVEL_INFO 3
#define LEVEL_DEBUG 4
#define LEVEL_TRACE 5

void set_log_level(int level);

void log_message(int level, const char *format, ...);

#endif //CTRONOME_LOGGING_ADAPTER_H
