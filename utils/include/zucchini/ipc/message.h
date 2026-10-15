#ifndef __ZUCCHINI_IPC_MESSAGE_H__
#define __ZUCCHINI_IPC_MESSAGE_H__

#include <stdint.h>

#include <zucchini/ipc/ring_buffer.h>

// Code 0 markiert in der Whitelist einen leeren Slot und ist damit als Ziel frei.
#define ZUC_MESSAGE_BROADCAST 0

typedef struct
{
    uint64_t code; // upstream: Ziel (ZUC_MESSAGE_BROADCAST = alle) / downstream: Absender
} ZUC_MessageHeader_t;

#define ZUC_MESSAGE_MAX_PAYLOAD (ZUC_RING_BUFFER_SLOT_SIZE - sizeof(ZUC_MessageHeader_t))

#endif
