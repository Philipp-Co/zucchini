#ifndef __ZUCCHINI_EVENTS_EVENT_H__
#define __ZUCCHINI_EVENTS_EVENT_H__

#include <stdint.h>

typedef enum
{
    ZUC_EVENT_WHITELIST_SOURCE_ADD    = 0,
    ZUC_EVENT_WHITELIST_SOURCE_REMOVE = 1,
} ZUC_EventType_t;

typedef struct
{
    uint64_t source_id;
} ZUC_WhitelistSourceAddEventPayload_t;

typedef struct
{
    uint64_t source_id;
} ZUC_WhitelistSourceRemoveEventPayload_t;

typedef union
{
    ZUC_WhitelistSourceAddEventPayload_t whitelist_source_add;
    ZUC_WhitelistSourceRemoveEventPayload_t whitelist_source_remove;
} ZUC_EventPayload_t;

typedef struct
{
    ZUC_EventType_t type;
    ZUC_EventPayload_t payload;
} ZUC_Event_t;

#endif
