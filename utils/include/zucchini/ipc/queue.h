#ifndef __ZUCCHINI_IPC_QUEUE_H__
#define __ZUCCHINI_IPC_QUEUE_H__

#include <stdint.h>

struct ZUC_IpcQueue;
typedef struct ZUC_IpcQueue* ZUC_IpcQueue_t;

typedef struct
{
    char data[1024];
    uint16_t capacity;
    uint16_t size;
} ZUC_IpcQueueItem_t;

typedef struct
{
    ZUC_IpcQueueItem_t item;
    int32_t result;
} ZUC_IpcQueueResult_t;

ZUC_IpcQueue_t ZUC_CreateIpcQueue(const char *name);
void ZUC_DestroyIpcQueue(ZUC_IpcQueue_t *queue);

void ZUC_PushToIpcQueue(ZUC_IpcQueue_t queue, ZUC_IpcQueueItem_t item);
ZUC_IpcQueueResult_t ZUC_PopFromIpcQueue(ZUC_IpcQueue_t queue);

ZUC_IpcQueueItem_t ZUC_CreateIpcQueueImte(void);
void ZUC_FreeIpcQueueItem(ZUC_IpcQueueItem_t *item);
int32_t ZUC_IsEmpty(const ZUC_IpcQueueItem_t item);

#endif
