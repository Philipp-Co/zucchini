#ifndef __ZUCCHINI_IPC_SHARED_MEMORY_H__
#define __ZUCCHINI_IPC_SHARED_MEMORY_H__

#include <stddef.h>

struct ZUC_SharedMemory;
typedef struct ZUC_SharedMemory* ZUC_SharedMemory_t;

ZUC_SharedMemory_t ZUC_CreateSharedMemory(const char *name, size_t size);
void ZUC_DestroySharedMemory(ZUC_SharedMemory_t *shared_memory);

void *ZUC_SharedMemoryAddress(ZUC_SharedMemory_t shared_memory);

#endif
