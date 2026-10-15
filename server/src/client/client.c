#include <zucchini/client/client.h>
#include <zucchini/ipc/ring_buffer.h>
#include <zucchini/ipc/message.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define ZUC_CLIENT_WAKEUP_PATH_PREFIX "/tmp/.zucchini_wakeup_"
#define ZUC_CLIENT_DOWNSTREAM_SUFFIX "_downstream"
#define ZUC_CLIENT_UPSTREAM_SUFFIX "_upstream"

struct ZUC_ClientInterface
{
    ZUC_RingBuffer_t send_ring_buffer;
    ZUC_RingBuffer_t receive_ring_buffer;
    int wakeup_filedescriptor;
    char wakeup_path[256];
};


ZUC_ClientInterface_t ZUC_CreateClientInterface(const char *name)
{
    ZUC_ClientInterface_t interface = malloc(sizeof(struct ZUC_ClientInterface));

    char send_name[256];
    snprintf(send_name, sizeof(send_name), "%s%s", name, ZUC_CLIENT_DOWNSTREAM_SUFFIX);
    interface->send_ring_buffer = ZUC_CreateRingBuffer(send_name);

    char receive_name[256];
    snprintf(receive_name, sizeof(receive_name), "%s%s", name, ZUC_CLIENT_UPSTREAM_SUFFIX);
    interface->receive_ring_buffer = ZUC_CreateRingBuffer(receive_name);

    interface->wakeup_filedescriptor = -1;
    snprintf(interface->wakeup_path, sizeof(interface->wakeup_path), "%s%s", ZUC_CLIENT_WAKEUP_PATH_PREFIX, name);

    if(mkfifo(interface->wakeup_path, 0666) == -1 && errno != EEXIST)
    {
        printf("Unable to create wakeup FIFO '%s': %s (%i)\n", interface->wakeup_path, strerror(errno), errno);
    }

    // Kein Oeffnungsversuch hier: solange kein Client die FIFO zum Lesen geoeffnet
    // hat, wuerde O_WRONLY|O_NONBLOCK sofort mit ENXIO fehlschlagen. Das Oeffnen
    // wird stattdessen bei jedem Send lazy nachgeholt, sobald ein Client verbunden ist.

    return interface;
}

void ZUC_DestroyClientInterface(ZUC_ClientInterface_t *interface)
{
    if((*interface)->wakeup_filedescriptor != -1)
    {
        close((*interface)->wakeup_filedescriptor);
    }
    ZUC_DestroyRingBuffer(&(*interface)->send_ring_buffer);
    ZUC_DestroyRingBuffer(&(*interface)->receive_ring_buffer);
    free(*interface);
    *interface = NULL;
}

int32_t ZUC_ClientInterfaceSend(ZUC_ClientInterface_t interface, uint64_t source, const void *data, uint16_t size)
{
    if(size > ZUC_MESSAGE_MAX_PAYLOAD)
    {
        return -1;
    }

    uint8_t message[ZUC_RING_BUFFER_SLOT_SIZE];
    const ZUC_MessageHeader_t header = { .code = source };
    memcpy(message, &header, sizeof(header));
    memcpy(message + sizeof(header), data, size);

    const int32_t result = ZUC_RingBufferPush(interface->send_ring_buffer, message, sizeof(header) + size);
    if(result != 0)
    {
        return result;
    }

    if(interface->wakeup_filedescriptor == -1)
    {
        // Lazy: erst jetzt versuchen, falls inzwischen ein Client die FIFO zum
        // Lesen geoeffnet hat. ENXIO (noch kein Leser) ist dabei kein echter Fehler.
        interface->wakeup_filedescriptor = open(interface->wakeup_path, O_WRONLY | O_NONBLOCK);
    }

    if(interface->wakeup_filedescriptor != -1)
    {
        const uint8_t byte = 1;
        if(write(interface->wakeup_filedescriptor, &byte, 1) == -1)
        {
            // Client evtl. weg (Absturz/Neustart) - fd verwerfen, beim naechsten Send erneut versuchen.
            close(interface->wakeup_filedescriptor);
            interface->wakeup_filedescriptor = -1;
        }
    }

    return 0;
}

int32_t ZUC_ClientInterfaceReceive(ZUC_ClientInterface_t interface, uint64_t *out_target, void *out_data, uint16_t *out_size)
{
    uint8_t message[ZUC_RING_BUFFER_SLOT_SIZE];
    uint16_t message_size;
    if(0 != ZUC_RingBufferPop(interface->receive_ring_buffer, message, &message_size))
    {
        return -1;
    }

    if(message_size < sizeof(ZUC_MessageHeader_t))
    {
        return -1;
    }

    ZUC_MessageHeader_t header;
    memcpy(&header, message, sizeof(header));

    *out_target = header.code;
    *out_size = message_size - sizeof(header);
    memcpy(out_data, message + sizeof(header), *out_size);
    return 0;
}
