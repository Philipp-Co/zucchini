#ifndef __ZUCCHINI_ADMIN_H__
#define __ZUCCHINI_ADMIN_H__

#include <stdint.h>
#include <netinet/in.h>
#include <stdatomic.h>

#include <zucchini/utils/logger.h>
#include <zucchini/io/networking/upd.h>
#include <zucchini/ipc/queue.h>
#include <zucchini/events/event.h>
#include <zucchini/status/status.h>


#define ZUCCHINI_ADMIN_MAX_CLIENTS 4

typedef struct
{
    ZUC_ServerStatusHandle_t status_handle;
    ZUC_Logger_t logger;
} ZUC_Admin_t;


void ZUC_InitializeAdminAdrea(ZUC_Admin_t *area, const char *name, ZUC_Logger_t root);
void ZUC_HandleAdminQueueItem(ZUC_Admin_t *area, ZUC_IpcQueueItem_t item);

int32_t ZUC_HandleUdpDatagram(ZUC_Admin_t *area, const ZUC_UdpDatagram_t *datagram, uint64_t *out_source);


#endif

