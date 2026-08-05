#include <zucchini/event_handling/event_handling.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/event.h>

#define ZUC_EVENT_HANDLING_MAX_EVENTS 32

static int ZUC_event_handling_kqueue = -1;
static ZUC_Logger_t ZUC_event_handling_logger = NULL;


int32_t ZUC_CreateEventHandling(ZUC_Logger_t root_logger)
{
    ZUC_event_handling_logger = ZUC_CreateChildLogger(root_logger, "event-handling"); 

    ZUC_event_handling_kqueue = kqueue();
    if(ZUC_event_handling_kqueue == -1)
    {
        ZUC_LoggerError(ZUC_event_handling_logger, "Unable to create kqueue instance\n");
        return -1;
    }
    return 0;
}

void ZUC_DestroyEventHandling(void)
{
    close(ZUC_event_handling_kqueue);
    ZUC_event_handling_kqueue = -1;
}

static int16_t ZUC_TranslateTrigger(ZUC_EventTrigger_t trigger)
{
    switch(trigger)
    {
        case ZUC_EVENT_TRIGGER_SIGNAL:
            return EVFILT_SIGNAL;
        case ZUC_EVENT_TRIGGER_READ:
        default:
            return EVFILT_READ;
    }
}

static ZUC_EventTrigger_t ZUC_TranslateFilter(int16_t filter)
{
    switch(filter)
    {
        case EVFILT_SIGNAL:
            return ZUC_EVENT_TRIGGER_SIGNAL;
        case EVFILT_READ:
        default:
            return ZUC_EVENT_TRIGGER_READ;
    }
}

int32_t ZUC_EventHandlingObserve(int filedescriptor, ZUC_EventTrigger_t trigger)
{
    if(trigger == ZUC_EVENT_TRIGGER_SIGNAL && signal(filedescriptor, SIG_IGN) == SIG_ERR)
    {
        ZUC_LoggerError(ZUC_event_handling_logger, "Unable to ignore signal %i: %s (%i)\n", filedescriptor, strerror(errno), errno);
        return -1;
    }

    struct kevent event;
    EV_SET(&event, filedescriptor, ZUC_TranslateTrigger(trigger), EV_ADD | EV_CLEAR, 0, 0, NULL);
    if(kevent(ZUC_event_handling_kqueue, &event, 1, NULL, 0, NULL) == -1)
    {
        ZUC_LoggerError(ZUC_event_handling_logger, "Unable to attach for kqueue Event...\n");
        return -1;
    }
    return 0;
}

int32_t ZUC_EventHandlingWait(ZUC_EventHandlingResult_t *out_results, int32_t max_results, const struct timespec *timeout)
{
    struct kevent events[ZUC_EVENT_HANDLING_MAX_EVENTS];

    const int result = kevent(ZUC_event_handling_kqueue, NULL, 0, events, ZUC_EVENT_HANDLING_MAX_EVENTS, timeout);
    if(result == -1)
    {
        if(errno == EINTR)
        {
            return 0;
        }
        ZUC_LoggerError(ZUC_event_handling_logger, "Error while reading from kqueue: %s (%i)\n", strerror(errno), errno);
        return -1;
    }

    const int32_t count = (result < max_results) ? result : max_results;
    for(int32_t i = 0; i < count; ++i)
    {
        out_results[i].identifier = (int)events[i].ident;
        out_results[i].trigger = ZUC_TranslateFilter(events[i].filter);
    }
    return count;
}
