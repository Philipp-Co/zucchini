#include <zucchini/client/client.h>
#include <zucchini/ipc/ring_buffer.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <poll.h>

#define ZUC_CLIENT_WAKEUP_PATH_PREFIX "/tmp/.zucchini_wakeup_"
#define ZUC_CLIENT_DOWNSTREAM_SUFFIX "_downstream"
#define ZUC_CLIENT_UPSTREAM_SUFFIX "_upstream"

struct ZUC_Client
{
    ZUC_RingBuffer_t receive_ring_buffer;
    ZUC_RingBuffer_t send_ring_buffer;
    int wakeup_filedescriptor;
};


ZUC_Client_t ZUC_CreateClient(const char *name)
{
    ZUC_Client_t client = malloc(sizeof(struct ZUC_Client));

    char receive_name[256];
    snprintf(receive_name, sizeof(receive_name), "%s%s", name, ZUC_CLIENT_DOWNSTREAM_SUFFIX);
    client->receive_ring_buffer = ZUC_CreateRingBuffer(receive_name);

    char send_name[256];
    snprintf(send_name, sizeof(send_name), "%s%s", name, ZUC_CLIENT_UPSTREAM_SUFFIX);
    client->send_ring_buffer = ZUC_CreateRingBuffer(send_name);

    client->wakeup_filedescriptor = -1;

    char wakeup_path[256];
    snprintf(wakeup_path, sizeof(wakeup_path), "%s%s", ZUC_CLIENT_WAKEUP_PATH_PREFIX, name);

    if(mkfifo(wakeup_path, 0666) == -1 && errno != EEXIST)
    {
        printf("Unable to create wakeup FIFO '%s': %s (%i)\n", wakeup_path, strerror(errno), errno);
        return client;
    }

    // Zunaechst non-blocking oeffnen, damit open() nicht blockiert, falls der Server
    // die Schreibseite noch nicht geoeffnet hat.
    const int fd = open(wakeup_path, O_RDONLY | O_NONBLOCK);
    if(fd == -1)
    {
        printf("Unable to open wakeup FIFO '%s': %s (%i)\n", wakeup_path, strerror(errno), errno);
        return client;
    }

    const int flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1 || fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) == -1)
    {
        printf("Unable to set wakeup FIFO blocking: %s (%i)\n", strerror(errno), errno);
        close(fd);
        return client;
    }

    client->wakeup_filedescriptor = fd;
    return client;
}

void ZUC_DestroyClient(ZUC_Client_t *client)
{
    if((*client)->wakeup_filedescriptor != -1)
    {
        close((*client)->wakeup_filedescriptor);
    }
    ZUC_DestroyRingBuffer(&(*client)->receive_ring_buffer);
    ZUC_DestroyRingBuffer(&(*client)->send_ring_buffer);
    free(*client);
    *client = NULL;
}

int32_t ZUC_ClientWait(ZUC_Client_t client, int32_t timeout_ms)
{
    if(client->wakeup_filedescriptor == -1)
    {
        return -1;
    }

    struct pollfd pfd = {
        .fd = client->wakeup_filedescriptor,
        .events = POLLIN,
        .revents = 0
    };

    const int result = poll(&pfd, 1, timeout_ms);
    if(result <= 0)
    {
        return -1;
    }

    uint8_t byte;
    read(client->wakeup_filedescriptor, &byte, 1);
    return 0;
}

int32_t ZUC_ClientReceive(ZUC_Client_t client, void *out_data, uint16_t *out_size)
{
    return ZUC_RingBufferPop(client->receive_ring_buffer, out_data, out_size);
}

int32_t ZUC_ClientSend(ZUC_Client_t client, const void *data, uint16_t size)
{
    return ZUC_RingBufferPush(client->send_ring_buffer, data, size);
}
