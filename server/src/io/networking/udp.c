#include <zucchini/io/networking/upd.h>
#include <zucchini/definitions.h>


#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>


ZUC_UdpServerSocket_t ZUC_CreateUdpServerSocket(uint16_t port, ZUC_Logger_t logger)
{
    ZUC_UdpServerSocket_t server = {
        .filedescriptor=-1,
        .logger=ZUC_CreateChildLogger(logger, "udp-server")
    };
    const int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd == -1)
    {
        ZUC_LoggerError(server.logger, "Unable to create UDP socket: %s (%i)\n", strerror(errno), errno);
        return server;
    }

    const int reuse = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
    {
        ZUC_LoggerError(server.logger, "Unable to set SO_REUSEADDR: %s (%i)\n", strerror(errno), errno);
        close(fd);
        return server;
    }

    const int flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        ZUC_LoggerError(server.logger, "Unable to set UDP socket non-blocking: %s (%i)\n", strerror(errno), errno);
        close(fd);
        return server;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if(bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        ZUC_LoggerError(server.logger, "Unable to bind UDP socket to port %i: %s (%i)\n", (int)port, strerror(errno), errno);
        close(fd);
        return server;
    }

    server.filedescriptor = fd;
    return server;
}

int32_t ZUC_UdpRecvFrom(ZUC_UdpServerSocket_t *socket, ZUC_UdpDatagram_t *datagram_buffer)
{
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);

    const ssize_t n = recvfrom(
        socket->filedescriptor,
        datagram_buffer->data, datagram_buffer->capacity,
        0,
        (struct sockaddr *)&sender, &sender_len
    );
    if(ZUC_UNLIKELY(n < 0))
    {
        datagram_buffer->size = 0;
        goto error;
    }
    else
    {
        datagram_buffer->source_address = sender;
        datagram_buffer->size = (uint16_t)n;
    }
    return 0;
error:
    return -1;
}

int32_t ZUC_UdpSendTo(ZUC_UdpServerSocket_t *socket, const struct sockaddr_in *destination, const uint8_t *data, uint16_t size)
{
    const ssize_t n = sendto(
        socket->filedescriptor,
        data, size,
        0,
        (const struct sockaddr *)destination, sizeof(*destination)
    );
    if(n < 0)
    {
        ZUC_LoggerError(socket->logger, "Unable to send UDP datagram: %s (%i)\n", strerror(errno), errno);
        return -1;
    }
    return 0;
}

