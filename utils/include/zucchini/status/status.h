#ifndef __ZUCCHINI_STATUS_STATUS_H__
#define __ZUCCHINI_STATUS_STATUS_H__

#include <stdint.h>
#include <sys/types.h>
#include <netinet/in.h>

#define ZUC_SERVER_STATUS_MAX_WHITELIST_ENTRIES 4

typedef struct
{
    uint64_t code;
    struct sockaddr_in address;
} ZUC_WhitelistEntry_t;

struct ZUC_ServerStatusHandle;
typedef struct ZUC_ServerStatusHandle* ZUC_ServerStatusHandle_t;

ZUC_ServerStatusHandle_t ZUC_CreateServerStatus(const char *name);
void ZUC_DestroyServerStatus(ZUC_ServerStatusHandle_t *handle);

pid_t ZUC_ServerStatusGetPid(ZUC_ServerStatusHandle_t handle);
void ZUC_ServerStatusSetPid(ZUC_ServerStatusHandle_t handle, pid_t pid);

void ZUC_ServerStatusWhitelistAdd(ZUC_ServerStatusHandle_t handle, uint64_t code);
void ZUC_ServerStatusWhitelistRemove(ZUC_ServerStatusHandle_t handle, uint64_t code);
int32_t ZUC_ServerStatusWhitelistContains(ZUC_ServerStatusHandle_t handle, uint64_t code);
void ZUC_ServerStatusWhitelistRecordSource(ZUC_ServerStatusHandle_t handle, uint64_t code, struct sockaddr_in address);
void ZUC_ServerStatusGetWhitelist(ZUC_ServerStatusHandle_t handle, ZUC_WhitelistEntry_t out_whitelist[ZUC_SERVER_STATUS_MAX_WHITELIST_ENTRIES]);

#endif
