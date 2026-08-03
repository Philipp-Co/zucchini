#ifndef __ZUCCHINI_EVENT_HANDLING_H__
#define __ZUCCHINI_EVENT_HANDLING_H__

#include <stdint.h>
#include <time.h>
#include <zucchini/utils/logger.h>

typedef enum
{
    ZUC_EVENT_TRIGGER_READ = 0,
} ZUC_EventTrigger_t;

int32_t ZUC_CreateEventHandling(ZUC_Logger_t root_logger);
void ZUC_DestroyEventHandling(void);

int32_t ZUC_EventHandlingObserve(int filedescriptor, ZUC_EventTrigger_t trigger);
int32_t ZUC_EventHandlingWait(int *out_filedescriptors, int32_t max_filedescriptors, const struct timespec *timeout);

#endif
