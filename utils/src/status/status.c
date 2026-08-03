#include <zucchini/status/status.h>
#include <zucchini/ipc/shared_memory.h>
#include <zucchini/ipc/pmutex.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct
{
    pid_t pid;
    ZUC_WhitelistEntry_t whitelist[ZUC_SERVER_STATUS_MAX_WHITELIST_ENTRIES];
} ZUC_ServerStatus_t;

struct ZUC_ServerStatusHandle
{
    ZUC_SharedMemory_t shared_memory;
    ZUC_ServerStatus_t *status;
    ZUC_PMutex_t mutex;
};


ZUC_ServerStatusHandle_t ZUC_CreateServerStatus(const char *name)
{
    ZUC_ServerStatusHandle_t handle = malloc(sizeof(struct ZUC_ServerStatusHandle));
    handle->shared_memory = ZUC_CreateSharedMemory(name, sizeof(ZUC_ServerStatus_t));
    if(NULL == handle->shared_memory)
    {
        printf("Unable to create shared Memory.\n");
        free(handle);
        return NULL;
    }
    handle->status = (ZUC_ServerStatus_t *)ZUC_SharedMemoryAddress(handle->shared_memory);
    if(NULL == handle->status)
    {
        free(handle);
        printf("Unable to create shared Memory, no Address assigned...\n");
        return NULL;
    }
    handle->mutex = ZUC_CreatePMutex(name);
    if(NULL == handle->mutex)
    {
        free(handle);
        printf("unable to create Mutex!\n");
        return NULL;
    }
    return handle;
}

void ZUC_DestroyServerStatus(ZUC_ServerStatusHandle_t *handle)
{
    ZUC_DestroyPMutex(&(*handle)->mutex);
    ZUC_DestroySharedMemory(&(*handle)->shared_memory);
    free(*handle);
    *handle = NULL;
}

pid_t ZUC_ServerStatusGetPid(ZUC_ServerStatusHandle_t handle)
{
    if(!handle->status)
    {
        return -1;
    }
    ZUC_PMutexLock(handle->mutex);
    const pid_t pid = handle->status->pid;
    ZUC_PMutexUnlock(handle->mutex);
    return pid;
}

void ZUC_ServerStatusSetPid(ZUC_ServerStatusHandle_t handle, pid_t pid)
{
    assert(NULL != handle);
    assert(NULL != handle->mutex);
    assert(NULL != handle->status);

    ZUC_PMutexLock(handle->mutex);
    handle->status->pid = pid;
    ZUC_PMutexUnlock(handle->mutex);
}

void ZUC_ServerStatusWhitelistAdd(ZUC_ServerStatusHandle_t handle, uint64_t code)
{
    if(!handle->status)
    {
        printf("Error: %p\n", handle->status);
        return;
    }

    ZUC_PMutexLock(handle->mutex);

    int32_t already_present = 0;
    for(int i = 0; i < ZUC_SERVER_STATUS_MAX_WHITELIST_ENTRIES; ++i)
    {
        if(handle->status->whitelist[i].code == code)
        {
            already_present = 1;
            break;
        }
    }

    if(!already_present)
    {
        for(int i = 0; i < ZUC_SERVER_STATUS_MAX_WHITELIST_ENTRIES; ++i)
        {
            if(handle->status->whitelist[i].code == 0)
            {
                handle->status->whitelist[i].code = code;
                memset(&handle->status->whitelist[i].address, 0, sizeof(handle->status->whitelist[i].address));
                break;
            }
        }
    }

    ZUC_PMutexUnlock(handle->mutex);
}

void ZUC_ServerStatusWhitelistRemove(ZUC_ServerStatusHandle_t handle, uint64_t code)
{
    if(!handle->status)
    {
        return;
    }

    ZUC_PMutexLock(handle->mutex);
    for(int i = 0; i < ZUC_SERVER_STATUS_MAX_WHITELIST_ENTRIES; ++i)
    {
        if(handle->status->whitelist[i].code == code)
        {
            handle->status->whitelist[i].code = 0;
            memset(&handle->status->whitelist[i].address, 0, sizeof(handle->status->whitelist[i].address));
            break;
        }
    }
    ZUC_PMutexUnlock(handle->mutex);
}

void ZUC_ServerStatusWhitelistRecordSource(ZUC_ServerStatusHandle_t handle, uint64_t code, struct sockaddr_in address)
{
    if(!handle->status)
    {
        return;
    }

    ZUC_PMutexLock(handle->mutex);
    for(int i = 0; i < ZUC_SERVER_STATUS_MAX_WHITELIST_ENTRIES; ++i)
    {
        if(handle->status->whitelist[i].code == code)
        {
            handle->status->whitelist[i].address = address;
            break;
        }
    }
    ZUC_PMutexUnlock(handle->mutex);
}

void ZUC_ServerStatusGetWhitelist(ZUC_ServerStatusHandle_t handle, ZUC_WhitelistEntry_t out_whitelist[ZUC_SERVER_STATUS_MAX_WHITELIST_ENTRIES])
{
    if(!handle->status)
    {
        memset(out_whitelist, 0, sizeof(ZUC_WhitelistEntry_t) * ZUC_SERVER_STATUS_MAX_WHITELIST_ENTRIES);
        return;
    }

    ZUC_PMutexLock(handle->mutex);
    memcpy(out_whitelist, handle->status->whitelist, sizeof(ZUC_WhitelistEntry_t) * ZUC_SERVER_STATUS_MAX_WHITELIST_ENTRIES);
    ZUC_PMutexUnlock(handle->mutex);
}

int32_t ZUC_ServerStatusWhitelistContains(ZUC_ServerStatusHandle_t handle, uint64_t code)
{
    if(!handle->status)
    {
        return 0;
    }

    ZUC_PMutexLock(handle->mutex);
    int32_t found = 0;
    for(int i = 0; i < ZUC_SERVER_STATUS_MAX_WHITELIST_ENTRIES; ++i)
    {
        if(handle->status->whitelist[i].code == code)
        {
            found = 1;
            break;
        }
    }
    ZUC_PMutexUnlock(handle->mutex);
    return found;
}
