#ifndef __ZUCCHINI_SHARED_LIBRARY_H__
#define __ZUCCHINI_SHARED_LIBRARY_H__

#include <stdint.h>

typedef struct
{
    uint16_t size;
    uint16_t capacity;
    uint8_t *data;
} ZUC_UdpBuffer_t;

typedef struct
{
    uint16_t size;
    uint16_t capacity;
    uint8_t *data;
} ZUC_TcpBuffer_t;

typedef ZUC_UdpBuffer_t (*ZUC_AllocateUdpBuffer_t)(uint16_t size);
typedef void (*ZUC_FreeUdpBuffer_t)(ZUC_UdpBuffer_t *buffer);

typedef ZUC_TcpBuffer_t (*ZUC_AllocateTcpBuffer_t)(uint16_t size);
typedef void (*ZUC_FreeTcpBuffer_t)(ZUC_TcpBuffer_t *buffer);

typedef struct
{
    ZUC_AllocateUdpBuffer_t allocate_udp_buffer;
    ZUC_FreeUdpBuffer_t free_udp_buffer;
    ZUC_AllocateTcpBuffer_t allocate_tcp_buffer;
    ZUC_FreeTcpBuffer_t free_tcp_buffer;
} ZUC_SharedLibraryCallbacks_t;

typedef int32_t (*ZUC_InitSharedLibrary)(ZUC_SharedLibraryCallbacks_t callbacks);
typedef int32_t (*ZUC_SharedLibraryUdpRequest_t)(const ZUC_UdpBuffer_t *input); 

typedef struct
{
    ZUC_TcpBuffer_t buffer;
    int32_t result;
} ZUC_TcpResponse_t;
typedef ZUC_TcpResponse_t (*ZUC_SharedLibraryTcpRequest_t)(const ZUC_TcpBuffer_t *input); 



#endif
