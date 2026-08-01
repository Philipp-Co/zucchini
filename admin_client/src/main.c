#include <zucchini/admin_client/commands.h>
#include <zucchini/ipc/queue.h>

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("Usage: %s <command> [args...]\n", argv[0]);
        return 1;
    }

    const ZUC_AdminCommand_t *command = NULL;
    for(uint32_t i = 0; i < ZUC_ADMIN_COMMAND_COUNT; ++i)
    {
        if(strcmp(ZUC_ADMIN_COMMANDS[i].name, argv[1]) == 0)
        {
            command = &ZUC_ADMIN_COMMANDS[i];
            break;
        }
    }

    if(!command)
    {
        printf("Unknown command '%s'\n", argv[1]);
        return 1;
    }

    ZUC_IpcQueue_t queue = ZUC_CreateIpcQueue(ZUC_ADMIN_SERVER_NAME);
    const int32_t result = command->execute(queue, argc - 2, argv + 2);
    ZUC_DestroyIpcQueue(&queue);

    return (result == 0) ? 0 : 1;
}
