#ifndef __ZUCCHINI_IO_TCP_H__
#define __ZUCCHINI_IO_TCP_H__


#include <stdint.h>


typedef struct 
{
    int filedescriptor;
} ZUC_TcpSocket_t;

typedef struct 
{
    int filedescriptor;
} ZUC_TcpServerSocket_t;

typedef struct
{
    uint8_t *data;
    uint16_t size;
    uint16_t capacity;
} ZUC_TcpMessage_t;

ZUC_TcpServerSocket_t ZUC_CreateTcpServerSocket(uint16_t port);
void ZUC_DestroyTcpServerSocket(ZUC_TcpServerSocket_t *socket);

ZUC_TcpSocket_t ZUC_TcpAccept(ZUC_TcpServerSocket_t *socket);
void ZUC_DestroyTcpSocket(ZUC_TcpSocket_t *socket);

int32_t ZUC_TcpReceive(ZUC_TcpSocket_t *socket, ZUC_TcpMessage_t *message);
int32_t ZUC_TcpSend(ZUC_TcpSocket_t *socket, ZUC_TcpMessage_t *message);


#endif

