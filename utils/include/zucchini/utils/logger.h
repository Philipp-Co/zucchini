#ifndef __ZUCCHINI_UTILS_LOGGER_H__
#define __ZUCCHINI_UTILS_LOGGER_H__

struct ZUC_Logger;
typedef struct ZUC_Logger* ZUC_Logger_t;

ZUC_Logger_t ZUC_CreateLogger(const char *name);
ZUC_Logger_t ZUC_CreateChildLogger(ZUC_Logger_t parent, const char *name);
void ZUC_DestroyLogger(ZUC_Logger_t *logger);

void ZUC_LoggerInfo(ZUC_Logger_t logger, const char *format, ...);
void ZUC_LoggerDebug(ZUC_Logger_t logger, const char *format, ...);
void ZUC_LoggerError(ZUC_Logger_t logger, const char *format, ...);
void ZUC_LoggerCritical(ZUC_Logger_t logger, const char *format, ...);

#endif
