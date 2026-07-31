#ifndef __ZUCCHINI_INPUT_H__
#define __ZUCCHINI_INPUT_H__

#include <stdint.h>


#define ZUC_IPC_MESSAGE_TYPE_SET_USER_ID 1U

typedef struct 
{
    uint64_t user_id;
} ZUC_IpcRequestSetUserId_t;

typedef struct
{

} ZUC_IpcResponseSetUserId_t;

typedef struct
{
    uint32_t message_id;
    uint8_t message_type;
    union
    {
        ZUC_IpcRequestSetUserId_t set_user_id;
    };
} ZUC_IpcRequest_t;

typedef struct
{
    uint32_t message_id;
    union
    {
        ZUC_IpcResponseSetUserId_t set_user_id;
    };
} ZUC_IpcResponse_t;


#endif
