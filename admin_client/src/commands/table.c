#include <zucchini/admin_client/commands.h>

const ZUC_AdminCommand_t ZUC_ADMIN_COMMANDS[] = {
    { "whitelist-add", "whitelist-add <hex-source-id>", ZUC_AdminCommand_WhitelistAdd },
    { "whitelist-remove", "whitelist-remove <hex-source-id>", ZUC_AdminCommand_WhitelistRemove },
    { "status", "status", ZUC_AdminCommand_Status },
    { "help", "help", ZUC_AdminCommand_Help },
};
const uint32_t ZUC_ADMIN_COMMAND_COUNT = sizeof(ZUC_ADMIN_COMMANDS) / sizeof(ZUC_ADMIN_COMMANDS[0]);
