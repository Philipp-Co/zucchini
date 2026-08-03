#include <zucchini/ipc/queue.h>

#include "key.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define ZUC_IPC_QUEUE_MESSAGE_TYPE 1

typedef struct
{
    long mtype;
    char mtext[1024];
} ZUC_SysVMessage_t;

struct ZUC_IpcQueue
{
    int msgid;
};


ZUC_IpcQueue_t ZUC_CreateIpcQueue(const char *name)
{
    ZUC_IpcQueue_t queue = malloc(sizeof(struct ZUC_IpcQueue));
    queue->msgid = -1;

    const key_t key = ZUC_IpcKeyFromName(name);
    if(key == -1)
    {
        printf("Unable to derive key for IPC queue '%s': %s (%i)\n", name, strerror(errno), errno);
        return queue;
    }

    const int msgid = msgget(key, IPC_CREAT | 0666);
    if(msgid == -1)
    {
        printf("Unable to create/open IPC queue '%s': %s (%i)\n", name, strerror(errno), errno);
        return queue;
    }

    queue->msgid = msgid;
    return queue;
}

void ZUC_DestroyIpcQueue(ZUC_IpcQueue_t *queue)
{
    // Die Queue bleibt im Kernel bestehen - Abraeumen ist Aufgabe eines anderen Prozesses.
    free(*queue);
    *queue = NULL;
}

void ZUC_PushToIpcQueue(ZUC_IpcQueue_t queue, ZUC_IpcQueueItem_t item)
{
    ZUC_SysVMessage_t message;
    message.mtype = ZUC_IPC_QUEUE_MESSAGE_TYPE;
    memcpy(message.mtext, item.data, item.size);

    if(msgsnd(queue->msgid, &message, item.size, IPC_NOWAIT) == -1)
    {
        printf("Unable to push to IPC queue: %s (%i)\n", strerror(errno), errno);
    }
}

ZUC_IpcQueueResult_t ZUC_PopFromIpcQueue(ZUC_IpcQueue_t queue)
{
    ZUC_IpcQueueItem_t item = ZUC_CreateIpcQueueImte();
    int32_t return_value = -1;
    ZUC_SysVMessage_t message;

    const ssize_t n = msgrcv(queue->msgid, &message, sizeof(message.mtext), ZUC_IPC_QUEUE_MESSAGE_TYPE, IPC_NOWAIT);
    if(n == -1)
    {
        if(errno != ENOMSG)
        {
            printf("Unable to pop from IPC queue: %s (%i)\n", strerror(errno), errno);
        }
        item.size = 0;
    }
    else
    {
        memcpy(item.data, message.mtext, (size_t)n);
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
        .capacity=1024,
        .size=0
    };
    return item;
}

void ZUC_FreeIpcQueueItem(ZUC_IpcQueueItem_t *item)
{
    assert(NULL != item);
    item->size = 0U;
}

int32_t ZUC_IsEmpty(const ZUC_IpcQueueItem_t item)
{
    return item.size == 0;
}
