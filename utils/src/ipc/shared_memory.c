#include <zucchini/ipc/shared_memory.h>

#include "key.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>

struct ZUC_SharedMemory
{
    int shmid;
    void *address;
};


ZUC_SharedMemory_t ZUC_CreateSharedMemory(const char *name, size_t size)
{
    ZUC_SharedMemory_t shared_memory = malloc(sizeof(struct ZUC_SharedMemory));
    shared_memory->shmid = -1;
    shared_memory->address = NULL;

    const key_t key = ZUC_IpcKeyFromName(name);
    if(key == -1)
    {
        printf("Unable to derive key for shared memory '%s': %s (%i)\n", name, strerror(errno), errno);
        free(shared_memory);
        return NULL;
    }

    const int shmid = shmget(key, size, IPC_CREAT | 0666);
    if(shmid == -1)
    {
        printf("Unable to create/open shared memory '%s': %s (%i)\n", name, strerror(errno), errno);
        free(shared_memory);
        return NULL;
    }
    shared_memory->shmid = shmid;

    void *address = shmat(shmid, NULL, 0);
    if(address == (void *)-1)
    {
        printf("Unable to attach shared memory '%s': %s (%i)\n", name, strerror(errno), errno);
        free(shared_memory);
        return NULL;
    }

    shared_memory->address = address;
    return shared_memory;
}

void ZUC_DestroySharedMemory(ZUC_SharedMemory_t *shared_memory)
{
    if((*shared_memory)->address != NULL)
    {
        shmdt((*shared_memory)->address);
    }
    // Das Segment selbst bleibt im Kernel bestehen - Abraeumen ist Aufgabe eines anderen Prozesses.
    free(*shared_memory);
    *shared_memory = NULL;
}

void *ZUC_SharedMemoryAddress(ZUC_SharedMemory_t shared_memory)
{
    return shared_memory->address;
}
