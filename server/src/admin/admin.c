#include <zucchini/admin/admin.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libkern/OSByteOrder.h>


void ZUC_InitializeAdminAdrea(ZUC_Admin_t *area, const char *name)
{
    area->status_handle = ZUC_CreateServerStatus(name);
    ZUC_ServerStatusSetPid(area->status_handle, getpid());
}

void ZUC_HandleAdminQueueItem(ZUC_Admin_t *area, ZUC_IpcQueueItem_t item)
{
    if(item.size != sizeof(ZUC_Event_t))
    {
        printf("Ignoring IPC queue item: unexpected size %u (expected %zu)\n", item.size, sizeof(ZUC_Event_t));
        return;
    }

    ZUC_Event_t event;
    memcpy(&event, item.data, sizeof(event));

    switch(event.type)
    {
        case ZUC_EVENT_WHITELIST_SOURCE_ADD:
            printf("Admin event: whitelist add source = 0x%llx\n", (unsigned long long)event.payload.whitelist_source_add.source_id);
            ZUC_ServerStatusWhitelistAdd(area->status_handle, event.payload.whitelist_source_add.source_id);
            break;
        case ZUC_EVENT_WHITELIST_SOURCE_REMOVE:
            printf("Admin event: whitelist remove source = 0x%llx\n", (unsigned long long)event.payload.whitelist_source_remove.source_id);
            ZUC_ServerStatusWhitelistRemove(area->status_handle, event.payload.whitelist_source_remove.source_id);
            break;
        default:
            printf("Unknown admin event type: %d\n", (int)event.type);
            break;
    }
}

int32_t ZUC_HandleUdpDatagram(ZUC_Admin_t *area, const ZUC_UdpDatagram_t *datagram)
{
    if(datagram->size < sizeof(uint64_t))
    {
        return -1;
    }

    uint64_t code;
    memcpy(&code, datagram->data, sizeof(code));
    code = OSSwapBigToHostInt64(code);

    if(!area->status_handle || !ZUC_ServerStatusWhitelistContains(area->status_handle, code))
    {
        return -1;
    }

    ZUC_ServerStatusWhitelistRecordSource(area->status_handle, code, datagram->source_address.sin_addr);
    return 0;
}
