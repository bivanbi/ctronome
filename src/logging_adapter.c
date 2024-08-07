#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "logging_adapter.h"

int log_level = LEVEL_INFO;

void set_log_level(int level) {
    log_level = level;
}

void log_message(int level, const char *format, ...) {
    va_list args;
    va_start(args, format);
    if (level > log_level) {
        return;
    }

    switch (level) {
        case LEVEL_FATAL:
            fprintf(stderr, "FATAL: ");
            break;
        case LEVEL_ERROR:
            fprintf(stderr, "ERROR: ");
            break;
        case LEVEL_WARNING:
            fprintf(stderr, "WARN: ");
            break;
        case LEVEL_INFO:
            fprintf(stderr, "INFO: ");
            break;
        case LEVEL_DEBUG:
            fprintf(stderr, "DEBUG: ");
            break;
        case LEVEL_TRACE:
            fprintf(stderr, "TRACE: ");
            break;
        default:
            fprintf(stderr, "UNKNOWN: ");
            break;
    }
    vfprintf(stderr, format, args);
    va_end(args);
}
