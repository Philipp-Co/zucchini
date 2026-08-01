#ifndef __ZUCCHINI_ADMIN_CLIENT_COMMANDS_H__
#define __ZUCCHINI_ADMIN_CLIENT_COMMANDS_H__

#include <stdint.h>

#include <zucchini/ipc/queue.h>

#define ZUC_ADMIN_SERVER_NAME "zucchini"

typedef int32_t (*ZUC_AdminCommandFn_t)(ZUC_IpcQueue_t queue, int argc, char **argv);

typedef struct
{
    const char *name;
    const char *usage;
    ZUC_AdminCommandFn_t execute;
} ZUC_AdminCommand_t;

extern const ZUC_AdminCommand_t ZUC_ADMIN_COMMANDS[];
extern const uint32_t ZUC_ADMIN_COMMAND_COUNT;

int32_t ZUC_AdminCommand_Send(ZUC_IpcQueue_t queue, int argc, char **argv);
int32_t ZUC_AdminCommand_WhitelistAdd(ZUC_IpcQueue_t queue, int argc, char **argv);
int32_t ZUC_AdminCommand_WhitelistRemove(ZUC_IpcQueue_t queue, int argc, char **argv);
int32_t ZUC_AdminCommand_Status(ZUC_IpcQueue_t queue, int argc, char **argv);
int32_t ZUC_AdminCommand_Help(ZUC_IpcQueue_t queue, int argc, char **argv);

#endif
