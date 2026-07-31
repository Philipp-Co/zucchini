#ifndef __ZUCCHINI_IO_UPD_H__
#define __ZUCCHINI_IO_UPD_H__

#include <stdint.h>
#include <netinet/in.h>

typedef struct
{
    int filedescriptor;
} ZUC_UdpServerSocket_t;

typedef struct
{
    struct sockaddr_in source_address;
    uint8_t *data;
    uint16_t size;
    uint16_t capacity;
} ZUC_UdpDatagram_t;

ZUC_UdpServerSocket_t ZUC_CreateUdpServerSocket(uint16_t port);
int32_t ZUC_UdpRecvFrom(ZUC_UdpServerSocket_t *socket, ZUC_UdpDatagram_t *datagram_buffer);


#endif
