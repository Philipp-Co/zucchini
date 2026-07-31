
#include <zucchini/io/networking/tcp.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>


ZUC_TcpServerSocket_t ZUC_CreateTcpServerSocket(uint16_t port)
{
    ZUC_TcpServerSocket_t server = {
        .filedescriptor=-1
    };

    const int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1)
    {
        goto error;
    }

    const int reuse = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
    {
        goto error_and_close;
    }

    const int flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        goto error_and_close;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if(bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        goto error_and_close;
    }

    if(listen(fd, SOMAXCONN) == -1)
    {
        goto error_and_close;
    }

    server.filedescriptor = fd;
error:
    return server;
error_and_close:
    close(fd);
    return server;
}

void ZUC_DestroyTcpServerSocket(ZUC_TcpServerSocket_t *socket)
{
    if(socket->filedescriptor != -1)
    {
        close(socket->filedescriptor);
        socket->filedescriptor = -1;
    }
}

ZUC_TcpSocket_t ZUC_TcpAccept(ZUC_TcpServerSocket_t *socket)
{
    ZUC_TcpSocket_t client_socket = {
        .filedescriptor=-1
    };

    const int fd = accept(socket->filedescriptor, NULL, NULL);
    if(fd == -1)
    {
        goto error;
    }

    const int flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        goto error_and_close;
    }
    
    client_socket.filedescriptor = fd;

error:
    return client_socket;
error_and_close:
    close(fd);
    return client_socket;
}

void ZUC_DestroyTcpSocket(ZUC_TcpSocket_t *socket)
{
    if(socket->filedescriptor != -1)
    {
        close(socket->filedescriptor);
        socket->filedescriptor = -1;
    }
}

int32_t ZUC_TcpReceive(ZUC_TcpSocket_t *socket, ZUC_TcpMessage_t *message)
{
    const ssize_t n = recv(socket->filedescriptor, message->data, message->capacity, 0);
    if(n <= 0)
    {
        message->size = 0;
        return -1;
    }
    else
    {
        message->size = (uint16_t)n;
        return 0;
    }
}

int32_t ZUC_TcpSend(ZUC_TcpSocket_t *socket, ZUC_TcpMessage_t *message)
{
    uint16_t total_sent = 0;
    message->size = 0;
    while(total_sent < message->size)
    {
        const ssize_t n = send(socket->filedescriptor, message->data + total_sent, message->size - total_sent, 0);
        if(n == -1)
        {
            goto error;
        }
        total_sent += (uint16_t)n;
    }
    return 0;
error:
    return -1;
}


