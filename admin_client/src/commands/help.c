#include <zucchini/admin_client/commands.h>

#include <stdio.h>

int32_t ZUC_AdminCommand_Help(const char *server_name, ZUC_IpcQueue_t queue, int argc, char **argv)
{
    (void)server_name;
    (void)queue;
    (void)argc;
    (void)argv;

    printf("Available commands:\n");
    for(uint32_t i = 0; i < ZUC_ADMIN_COMMAND_COUNT; ++i)
    {
        printf("  %s\n", ZUC_ADMIN_COMMANDS[i].usage);
    }
    return 0;
}
