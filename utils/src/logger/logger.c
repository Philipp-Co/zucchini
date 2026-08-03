#include <zucchini/utils/logger.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define ZUC_LOGGER_NAME_MAX_LENGTH 64

struct ZUC_Logger
{
    char name[ZUC_LOGGER_NAME_MAX_LENGTH];
};


ZUC_Logger_t ZUC_CreateLogger(const char *name)
{
    ZUC_Logger_t logger = malloc(sizeof(struct ZUC_Logger));
    snprintf(logger->name, sizeof(logger->name), "%s", name);
    return logger;
}

ZUC_Logger_t ZUC_CreateChildLogger(ZUC_Logger_t parent, const char *name)
{
    ZUC_Logger_t logger = malloc(sizeof(struct ZUC_Logger));
    snprintf(logger->name, sizeof(logger->name), "%s.%s", parent->name, name);
    return logger;
}

void ZUC_DestroyLogger(ZUC_Logger_t *logger)
{
    free(*logger);
    *logger = NULL;
}

static void ZUC_LoggerLog(ZUC_Logger_t logger, const char *level, const char *format, va_list args)
{
    printf("[%s][%s] - ", level, logger->name);
    vprintf(format, args);
    printf("\n");
}

void ZUC_LoggerInfo(ZUC_Logger_t logger, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    ZUC_LoggerLog(logger, "INFO", format, args);
    va_end(args);
}

void ZUC_LoggerDebug(ZUC_Logger_t logger, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    ZUC_LoggerLog(logger, "DEBUG", format, args);
    va_end(args);
}

void ZUC_LoggerError(ZUC_Logger_t logger, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    ZUC_LoggerLog(logger, "ERROR", format, args);
    va_end(args);
}

void ZUC_LoggerCritical(ZUC_Logger_t logger, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    ZUC_LoggerLog(logger, "CRITICAL", format, args);
    va_end(args);
}
