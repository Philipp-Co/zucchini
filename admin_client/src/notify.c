#include <zucchini/admin_client/notify.h>
#include <zucchini/admin_client/commands.h>
#include <zucchini/status/status.h>

#include <signal.h>
#include <unistd.h>

int32_t ZUC_AdminNotifyServer(const char *server_name)
{
    ZUC_ServerStatusHandle_t handle = ZUC_CreateServerStatus(server_name);
    if(!handle)
    {
        return -1;
    }

    const pid_t pid = ZUC_ServerStatusGetPid(handle);
    ZUC_DestroyServerStatus(&handle);

    if(pid <= 0)
    {
        return -1;
    }

    return kill(pid, SIGUSR1);
}
