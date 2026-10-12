#include <zucchini/admin_client/commands.h>
#include <zucchini/ipc/queue.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    const char *server_name = NULL;
    int option;
    while(-1 != (option = getopt(argc, argv, "n:")))
    {
        switch(option)
        {
            case 'n':
                server_name = optarg;
                break;
            default:
                printf("Usage: %s -n <name> <command> [args...]\n", argv[0]);
                return 1;
        }
    }

    if(server_name == NULL || server_name[0] == '\0' || optind >= argc)
    {
        printf("Usage: %s -n <name> <command> [args...]\n", argv[0]);
        return 1;
    }

    const char *command_name = argv[optind];
    const ZUC_AdminCommand_t *command = NULL;
    for(uint32_t i = 0; i < ZUC_ADMIN_COMMAND_COUNT; ++i)
    {
        if(strcmp(ZUC_ADMIN_COMMANDS[i].name, command_name) == 0)
        {
            command = &ZUC_ADMIN_COMMANDS[i];
            break;
        }
    }

    if(!command)
    {
        printf("Unknown command '%s'\n", command_name);
        return 1;
    }

    ZUC_IpcQueue_t queue = ZUC_CreateIpcQueue(server_name);
    const int32_t result = command->execute(server_name, queue, argc - optind - 1, argv + optind + 1);
    ZUC_DestroyIpcQueue(&queue);

    return (result == 0) ? 0 : 1;
}
