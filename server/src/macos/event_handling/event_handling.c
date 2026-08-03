#include <zucchini/event_handling/event_handling.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/event.h>

#define ZUC_EVENT_HANDLING_MAX_EVENTS 32

static int ZUC_event_handling_kqueue = -1;


int32_t ZUC_CreateEventHandling(void)
{
    ZUC_event_handling_kqueue = kqueue();
    if(ZUC_event_handling_kqueue == -1)
    {
        printf("Unable to create kqueue instance\n");
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
        case ZUC_EVENT_TRIGGER_READ:
        default:
            return EVFILT_READ;
    }
}

int32_t ZUC_EventHandlingObserve(int filedescriptor, ZUC_EventTrigger_t trigger)
{
    struct kevent event;
    EV_SET(&event, filedescriptor, ZUC_TranslateTrigger(trigger), EV_ADD | EV_CLEAR, 0, 0, NULL);
    if(kevent(ZUC_event_handling_kqueue, &event, 1, NULL, 0, NULL) == -1)
    {
        printf("Unable to attach for kqeueu Event...\n");
        return -1;
    }
    return 0;
}

int32_t ZUC_EventHandlingWait(int *out_filedescriptors, int32_t max_filedescriptors, const struct timespec *timeout)
{
    struct kevent events[ZUC_EVENT_HANDLING_MAX_EVENTS];

    const int result = kevent(ZUC_event_handling_kqueue, NULL, 0, events, ZUC_EVENT_HANDLING_MAX_EVENTS, timeout);
    if(result == -1)
    {
        if(errno == EINTR)
        {
            return 0;
        }
        printf("Error while reading from kqueue: %s (%i)\n", strerror(errno), errno);
        return -1;
    }

    const int32_t count = (result < max_filedescriptors) ? result : max_filedescriptors;
    for(int32_t i = 0; i < count; ++i)
    {
        out_filedescriptors[i] = (int)events[i].ident;
    }
    return count;
}
