#include <zucchini/admin_client/commands.h>
#include <zucchini/admin_client/hex.h>
#include <zucchini/admin_client/notify.h>
#include <zucchini/events/event.h>

#include <stdio.h>
#include <string.h>

int32_t ZUC_AdminCommand_WhitelistRemove(ZUC_IpcQueue_t queue, int argc, char **argv)
{
    if(argc < 1)
    {
        printf("Usage: whitelist-remove <hex-source-id>\n");
        return -1;
    }

    uint64_t source_id;
    if(-1 == ZUC_ParseHexUint64(argv[0], &source_id))
    {
        printf("Invalid hexadecimal value: %s\n", argv[0]);
        return -1;
    }

    ZUC_Event_t event = { .type = ZUC_EVENT_WHITELIST_SOURCE_REMOVE };
    event.payload.whitelist_source_remove.source_id = source_id;

    ZUC_IpcQueueItem_t item = ZUC_CreateIpcQueueImte();
    memcpy(item.data, &event, sizeof(event));
    item.size = (uint16_t)sizeof(event);

    ZUC_PushToIpcQueue(queue, item);
    ZUC_AdminNotifyServer();
    return 0;
}
