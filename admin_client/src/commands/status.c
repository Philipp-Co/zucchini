#include <zucchini/admin_client/commands.h>
#include <zucchini/status/status.h>

#include <stdio.h>
#include <arpa/inet.h>

int32_t ZUC_AdminCommand_Status(ZUC_IpcQueue_t queue, int argc, char **argv)
{
    (void)queue;
    (void)argc;
    (void)argv;

    ZUC_ServerStatusHandle_t handle = ZUC_CreateServerStatus(ZUC_ADMIN_SERVER_NAME);

    ZUC_WhitelistEntry_t whitelist[ZUC_SERVER_STATUS_MAX_WHITELIST_ENTRIES];
    ZUC_ServerStatusGetWhitelist(handle, whitelist);

    printf("Server status:\n");
    printf("  PID: %d\n", (int)ZUC_ServerStatusGetPid(handle));
    printf("  Whitelist:\n");
    for(int i = 0; i < ZUC_SERVER_STATUS_MAX_WHITELIST_ENTRIES; ++i)
    {
        if(whitelist[i].code != 0)
        {
            printf("    [%d] 0x%llx  %s:%u\n", i, (unsigned long long)whitelist[i].code,
                inet_ntoa(whitelist[i].address.sin_addr), (unsigned)ntohs(whitelist[i].address.sin_port));
        }
    }

    ZUC_DestroyServerStatus(&handle);
    return 0;
}
