#include <zucchini/event_handling/event_handling.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/epoll.h>

#define ZUC_EVENT_HANDLING_MAX_EVENTS 32

static int ZUC_event_handling_epoll = -1;
static ZUC_Logger_t ZUC_event_handling_logger = NULL;


int32_t ZUC_CreateEventHandling(ZUC_Logger_t root_logger)
{
    ZUC_event_handling_logger = ZUC_CreateChildLogger(root_logger, "event-handling"); 
    ZUC_event_handling_epoll = epoll_create1(0);
    if(ZUC_event_handling_epoll == -1)
    {
        ZUC_LoggerError(ZUC_event_handling_logger, "Unable to create epoll instance: %s (%i)\n", strerror(errno), errno);
        return -1;
    }
    return 0;
}

void ZUC_DestroyEventHandling(void)
{
    close(ZUC_event_handling_epoll);
    ZUC_event_handling_epoll = -1;
}

static uint32_t ZUC_TranslateTrigger(ZUC_EventTrigger_t trigger)
{
    switch(trigger)
    {
        case ZUC_EVENT_TRIGGER_READ:
        default:
            return EPOLLIN;
    }
}

int32_t ZUC_EventHandlingObserve(int filedescriptor, ZUC_EventTrigger_t trigger)
{
    struct epoll_event event;
    event.events = ZUC_TranslateTrigger(trigger) | EPOLLET;
    event.data.fd = filedescriptor;

    if(epoll_ctl(ZUC_event_handling_epoll, EPOLL_CTL_ADD, filedescriptor, &event) == -1)
    {
        ZUC_LoggerInfo(ZUC_event_handling_logger, "Unable to attach for epoll event: %s (%i)\n", strerror(errno), errno);
        return -1;
    }
    return 0;
}

static int32_t ZUC_TimespecToMilliseconds(const struct timespec *timeout)
{
    if(timeout == NULL)
    {
        return -1;
    }
    return (int32_t)(timeout->tv_sec * 1000 + timeout->tv_nsec / 1000000);
}

int32_t ZUC_EventHandlingWait(int *out_filedescriptors, int32_t max_filedescriptors, const struct timespec *timeout)
{
    struct epoll_event events[ZUC_EVENT_HANDLING_MAX_EVENTS];

    const int result = epoll_wait(
        ZUC_event_handling_epoll, events, ZUC_EVENT_HANDLING_MAX_EVENTS, ZUC_TimespecToMilliseconds(timeout)
    );
    if(result == -1)
    {
        if(errno == EINTR)
        {
            return 0;
        }
        ZUC_LoggerError(ZUC_event_handling_logger, "Error while waiting on epoll: %s (%i)\n", strerror(errno), errno);
        return -1;
    }

    const int32_t count = (result < max_filedescriptors) ? result : max_filedescriptors;
    for(int32_t i = 0; i < count; ++i)
    {
        out_filedescriptors[i] = events[i].data.fd;
    }
    return count;
}
