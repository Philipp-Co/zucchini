#include <zucchini/api/api.h>
#include <zucchini/ipc/ring_buffer.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <poll.h>

#define ZUC_API_WAKEUP_PATH_PREFIX "/tmp/.zucchini_wakeup_"
#define ZUC_API_DOWNSTREAM_SUFFIX "_downstream"
#define ZUC_API_UPSTREAM_SUFFIX "_upstream"

struct ZUC_Api
{
    ZUC_RingBuffer_t receive_ring_buffer;
    ZUC_RingBuffer_t send_ring_buffer;
    int wakeup_filedescriptor;
};


ZUC_Api_t ZUC_CreateApi(const char *name)
{
    ZUC_Api_t api = malloc(sizeof(struct ZUC_Api));

    char receive_name[256];
    snprintf(receive_name, sizeof(receive_name), "%s%s", name, ZUC_API_DOWNSTREAM_SUFFIX);
    api->receive_ring_buffer = ZUC_CreateRingBuffer(receive_name);

    char send_name[256];
    snprintf(send_name, sizeof(send_name), "%s%s", name, ZUC_API_UPSTREAM_SUFFIX);
    api->send_ring_buffer = ZUC_CreateRingBuffer(send_name);

    api->wakeup_filedescriptor = -1;

    char wakeup_path[256];
    snprintf(wakeup_path, sizeof(wakeup_path), "%s%s", ZUC_API_WAKEUP_PATH_PREFIX, name);

    if(mkfifo(wakeup_path, 0666) == -1 && errno != EEXIST)
    {
        printf("Unable to create wakeup FIFO '%s': %s (%i)\n", wakeup_path, strerror(errno), errno);
        return api;
    }

    // Zunaechst non-blocking oeffnen, damit open() nicht blockiert, falls der Server
    // die Schreibseite noch nicht geoeffnet hat.
    const int fd = open(wakeup_path, O_RDONLY | O_NONBLOCK);
    if(fd == -1)
    {
        printf("Unable to open wakeup FIFO '%s': %s (%i)\n", wakeup_path, strerror(errno), errno);
        return api;
    }

    const int flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1 || fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) == -1)
    {
        printf("Unable to set wakeup FIFO blocking: %s (%i)\n", strerror(errno), errno);
        close(fd);
        return api;
    }

    api->wakeup_filedescriptor = fd;
    return api;
}

void ZUC_DestroyApi(ZUC_Api_t *api)
{
    if((*api)->wakeup_filedescriptor != -1)
    {
        close((*api)->wakeup_filedescriptor);
    }
    ZUC_DestroyRingBuffer(&(*api)->receive_ring_buffer);
    ZUC_DestroyRingBuffer(&(*api)->send_ring_buffer);
    free(*api);
    *api = NULL;
}

int32_t ZUC_ApiWait(ZUC_Api_t api, int32_t timeout_ms)
{
    if(api->wakeup_filedescriptor == -1)
    {
        return -1;
    }

    struct pollfd pfd = {
        .fd = api->wakeup_filedescriptor,
        .events = POLLIN,
        .revents = 0
    };

    const int result = poll(&pfd, 1, timeout_ms);
    if(result <= 0)
    {
        return -1;
    }

    uint8_t byte;
    read(api->wakeup_filedescriptor, &byte, 1);
    return 0;
}

int32_t ZUC_ApiReceive(ZUC_Api_t api, void *out_data, uint16_t *out_size)
{
    return ZUC_RingBufferPop(api->receive_ring_buffer, out_data, out_size);
}

int32_t ZUC_ApiSend(ZUC_Api_t api, const void *data, uint16_t size)
{
    return ZUC_RingBufferPush(api->send_ring_buffer, data, size);
}
