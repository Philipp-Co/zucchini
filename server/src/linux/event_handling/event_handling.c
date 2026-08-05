#include <zucchini/event_handling/event_handling.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>

#define ZUC_EVENT_HANDLING_MAX_EVENTS 32

static int ZUC_event_handling_epoll = -1;
static int ZUC_event_handling_signalfd = -1;
static sigset_t ZUC_event_handling_signal_mask;
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
    sigemptyset(&ZUC_event_handling_signal_mask);
    return 0;
}

void ZUC_DestroyEventHandling(void)
{
    if(ZUC_event_handling_signalfd != -1)
    {
        close(ZUC_event_handling_signalfd);
        ZUC_event_handling_signalfd = -1;
    }
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
    if(trigger == ZUC_EVENT_TRIGGER_SIGNAL)
    {
        sigaddset(&ZUC_event_handling_signal_mask, filedescriptor);
        if(sigprocmask(SIG_BLOCK, &ZUC_event_handling_signal_mask, NULL) == -1)
        {
            ZUC_LoggerError(ZUC_event_handling_logger, "Unable to block signal %i: %s (%i)\n", filedescriptor, strerror(errno), errno);
            return -1;
        }

        const int signal_fd = signalfd(ZUC_event_handling_signalfd, &ZUC_event_handling_signal_mask, SFD_NONBLOCK);
        if(signal_fd == -1)
        {
            ZUC_LoggerError(ZUC_event_handling_logger, "Unable to create signalfd: %s (%i)\n", strerror(errno), errno);
            return -1;
        }

        if(ZUC_event_handling_signalfd == -1)
        {
            ZUC_event_handling_signalfd = signal_fd;

            struct epoll_event event;
            event.events = EPOLLIN;
            event.data.fd = ZUC_event_handling_signalfd;

            if(epoll_ctl(ZUC_event_handling_epoll, EPOLL_CTL_ADD, ZUC_event_handling_signalfd, &event) == -1)
            {
                ZUC_LoggerError(ZUC_event_handling_logger, "Unable to attach signalfd to epoll: %s (%i)\n", strerror(errno), errno);
                return -1;
            }
        }
        return 0;
    }

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

int32_t ZUC_EventHandlingWait(ZUC_EventHandlingResult_t *out_results, int32_t max_results, const struct timespec *timeout)
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

    const int32_t count = (result < max_results) ? result : max_results;
    for(int32_t i = 0; i < count; ++i)
    {
        if(events[i].data.fd == ZUC_event_handling_signalfd)
        {
            struct signalfd_siginfo siginfo;
            const ssize_t n = read(ZUC_event_handling_signalfd, &siginfo, sizeof(siginfo));
            out_results[i].identifier = (n == (ssize_t)sizeof(siginfo)) ? (int)siginfo.ssi_signo : -1;
            out_results[i].trigger = ZUC_EVENT_TRIGGER_SIGNAL;
        }
        else
        {
            out_results[i].identifier = events[i].data.fd;
            out_results[i].trigger = ZUC_EVENT_TRIGGER_READ;
        }
    }
    return count;
}
