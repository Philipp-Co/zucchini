#include <zucchini/ipc/queue.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>


struct ZUC_IpcQueue
{
    int read_filedescriptor;
    int write_filedescriptor;
};


static int32_t ZUC_SetNonBlocking(int filedescriptor)
{
    const int flags = fcntl(filedescriptor, F_GETFL, 0);
    if(flags == -1 || fcntl(filedescriptor, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        printf("Unable to set IPC queue socket non-blocking: %s (%i)\n", strerror(errno), errno);
        return -1;
    }
    return 0;
}

int ZUC_IpcQueueFiledescriptor(ZUC_IpcQueue_t queue)
{
    return queue->read_filedescriptor;
}

ZUC_IpcQueue_t ZUC_CreateIpcQueue(void)
{
    ZUC_IpcQueue_t queue = malloc(sizeof(struct ZUC_IpcQueue));
    queue->read_filedescriptor = -1;
    queue->write_filedescriptor = -1;

    int fds[2];
    if(socketpair(AF_UNIX, SOCK_DGRAM, 0, fds) == -1)
    {
        printf("Unable to create IPC queue socketpair: %s (%i)\n", strerror(errno), errno);
        goto error;
    }

    if(-1 == ZUC_SetNonBlocking(fds[0]) || -1 == ZUC_SetNonBlocking(fds[1]))
    {
        close(fds[0]);
        close(fds[1]);
        goto error;
    }

    queue->read_filedescriptor = fds[0];
    queue->write_filedescriptor = fds[1];
    return queue;
error:
    free(queue);
    return NULL;
}

void ZUC_DestroyIpcQueue(ZUC_IpcQueue_t *queue)
{
    if((*queue)->read_filedescriptor != -1)
    {
        close((*queue)->read_filedescriptor);
    }
    if((*queue)->write_filedescriptor != -1)
    {
        close((*queue)->write_filedescriptor);
    }
    free(*queue);
    *queue = NULL;
}

void ZUC_PushToIpcQueue(ZUC_IpcQueue_t queue, ZUC_IpcQueueItem_t item)
{
    if(send(queue->write_filedescriptor, item.data, item.size, 0) == -1)
    {
        if(errno != EAGAIN && errno != EWOULDBLOCK)
        {
            printf("Unable to push to IPC queue: %s (%i)\n", strerror(errno), errno);
        }
    }
}

ZUC_IpcQueueResult_t ZUC_PopFromIpcQueue(ZUC_IpcQueue_t queue)
{
    ZUC_IpcQueueItem_t item = ZUC_CreateIpcQueueImte();
    int32_t return_value = -1;

    const ssize_t n = recv(queue->read_filedescriptor, item.data, item.capacity, 0);
    if(n <= 0)
    {
        if(n == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
        {
            printf("Unable to pop from IPC queue: %s (%i)\n", strerror(errno), errno);
        }
        item.size = 0;
    }
    else
    {
        item.size = (uint16_t)n;
        return_value = 0;
    }
    ZUC_IpcQueueResult_t result = {
        .item = item,
        .result = return_value
    };
    return result;
}

ZUC_IpcQueueItem_t ZUC_CreateIpcQueueImte(void)
{
    ZUC_IpcQueueItem_t item = {
        .data=malloc(1024),
        .capacity=1024,
        .size=0
    };
    return item;
}

void ZUC_FreeIpcQueueItem(ZUC_IpcQueueItem_t *item)
{
    assert(NULL != item);
    if(item->data)
    {
        free(item->data);
    }
    item->data = NULL;
    item->capacity = 0U;
    item->size = 0U;
}

int32_t ZUC_IsEmpty(const ZUC_IpcQueueItem_t item)
{
    return item.size == 0;
}
