#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define ZUC_REMOTE_CLIENT_RECV_TIMEOUT_SECONDS 2
#define ZUC_REMOTE_CLIENT_BUFFER_SIZE 1024

static void ZUC_EncodeBigEndianUint64(uint8_t *out, uint64_t value)
{
    for(int i = 0; i < 8; ++i)
    {
        out[i] = (uint8_t)(value >> (8 * (7 - i)));
    }
}

int main(int argc, char **argv)
{
    if(argc < 4)
    {
        printf("Usage: %s <host> <port> <code> [payload]\n", argv[0]);
        return 1;
    }

    const char *host = argv[1];
    const uint16_t port = (uint16_t)atoi(argv[2]);
    const uint64_t code = strtoull(argv[3], NULL, 0);
    const char *payload = (argc > 4) ? argv[4] : "";
    const size_t payload_size = strlen(payload);

    if(payload_size > ZUC_REMOTE_CLIENT_BUFFER_SIZE - 8)
    {
        printf("Payload too large\n");
        return 1;
    }

    uint8_t message[ZUC_REMOTE_CLIENT_BUFFER_SIZE];
    ZUC_EncodeBigEndianUint64(message, code);
    memcpy(message + 8, payload, payload_size);
    const size_t message_size = 8 + payload_size;

    const int filedescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if(filedescriptor == -1)
    {
        printf("Unable to create socket: %s (%i)\n", strerror(errno), errno);
        return 1;
    }

    struct sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    if(inet_pton(AF_INET, host, &server_address.sin_addr) != 1)
    {
        printf("Invalid host address '%s'\n", host);
        close(filedescriptor);
        return 1;
    }

    if(sendto(filedescriptor, message, message_size, 0, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        printf("Unable to send datagram: %s (%i)\n", strerror(errno), errno);
        close(filedescriptor);
        return 1;
    }
    printf("Sent %zu bytes to %s:%u (code=0x%llx)\n", message_size, host, port, (unsigned long long)code);

    struct timeval timeout = { .tv_sec = ZUC_REMOTE_CLIENT_RECV_TIMEOUT_SECONDS, .tv_usec = 0 };
    setsockopt(filedescriptor, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    uint8_t response[ZUC_REMOTE_CLIENT_BUFFER_SIZE];
    struct sockaddr_in from_address;
    socklen_t from_address_length = sizeof(from_address);
    const ssize_t received = recvfrom(filedescriptor, response, sizeof(response), 0, (struct sockaddr *)&from_address, &from_address_length);
    if(received == -1)
    {
        printf("No response received: %s (%i)\n", strerror(errno), errno);
    }
    else
    {
        printf("Received %zd bytes: %.*s\n", received, (int)received, (char *)response);
    }

    close(filedescriptor);
    return 0;
}
