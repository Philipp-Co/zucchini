
#include <stdio.h>
#include <stdatomic.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <err.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <zucchini/utils/logger.h>
#include <zucchini/server.h>
#include <zucchini/io/networking/upd.h>
#include <zucchini/admin/admin.h>
#include <zucchini/definitions.h>
#include <zucchini/ipc/queue.h>
#include <zucchini/status/status.h>
#include <zucchini/client/client.h>
#include <zucchini/event_handling/event_handling.h>


#define ZUC_UDP_SERVER_PORT 9999


typedef struct
{
    ZUC_Logger_t logger;

    ZUC_UdpServerSocket_t upd_server_socket;
    ZUC_UdpDatagram_t udp_datagram_buffer;

    ZUC_IpcQueue_t command_queue;
    ZUC_ClientInterface_t client_interface;

    ZUC_Admin_t admin_area;

    atomic_int terminate;

    uint8_t udp_message_buffer[1024];
} ZUC_Server_t;


static ZUC_Server_t ZUC_server = {0};


static void ZUC_EventLoop(ZUC_Server_t *server);
static int32_t ZUC_EventLoopHandleEvent(ZUC_Server_t *server, ZUC_EventHandlingResult_t event);


__attribute__((cold))
int32_t ZUC_CreateServer(const char *name)
{
    ZUC_server.logger = ZUC_CreateLogger("server");

    atomic_store(&ZUC_server.terminate, 0);

    if(-1 == ZUC_CreateEventHandling(ZUC_server.logger))
    {
        return -1;
    }

    ZUC_server.upd_server_socket = ZUC_CreateUdpServerSocket(ZUC_UDP_SERVER_PORT, ZUC_server.logger);
    if(-1 == ZUC_server.upd_server_socket.filedescriptor)
    {
        return -1;
    }
    ZUC_EventHandlingObserve(ZUC_server.upd_server_socket.filedescriptor, ZUC_EVENT_TRIGGER_READ);
    ZUC_EventHandlingObserve(SIGUSR1, ZUC_EVENT_TRIGGER_SIGNAL);
    ZUC_server.udp_datagram_buffer.capacity = sizeof(ZUC_server.udp_message_buffer);
    ZUC_server.udp_datagram_buffer.size = 0;
    ZUC_server.udp_datagram_buffer.data = ZUC_server.udp_message_buffer;

    ZUC_server.command_queue = ZUC_CreateIpcQueue(name);
    if(!ZUC_server.command_queue)
    {
        ZUC_LoggerInfo(ZUC_server.logger, "Unable to create Ipc Queue.\n");
        return -1;
    }

    ZUC_server.client_interface = ZUC_CreateClientInterface(name);

    ZUC_InitializeAdminAdrea(&ZUC_server.admin_area, name, ZUC_server.logger);
    return 0;
}

__attribute__((cold))
void ZUC_DestroyServer(void)
{
    ZUC_DestroyEventHandling();
}

__attribute__((hot))
void ZUC_RunServer(void)
{
    while(0 == atomic_load(&ZUC_server.terminate))
    {
        ZUC_EventLoop(&ZUC_server);
    }
}

__attribute__((cold))
void ZUC_TerminateServer(void)
{
    atomic_store(&ZUC_server.terminate, 1);
}

#define ZUCCHINI_EVENTLOOP_NUMBER_OF_EVENTS 32
__attribute__((hot))
static void ZUC_EventLoop(ZUC_Server_t *server)
{
    ZUC_EventHandlingResult_t events[ZUCCHINI_EVENTLOOP_NUMBER_OF_EVENTS];
    struct timespec timeout = {
        .tv_sec=0,
        .tv_nsec=1000 * 1000 * 5
    };
    const int32_t result = ZUC_EventHandlingWait(events, ZUCCHINI_EVENTLOOP_NUMBER_OF_EVENTS, &timeout);
    for(int i=0;i<result;++i)
    {
        ZUC_EventLoopHandleEvent(&ZUC_server, events[i]);
    }
    
    uint8_t client_message[1024];
    uint16_t client_message_size;
    while(1)
    {
        if(0 == ZUC_ClientInterfaceReceive(server->client_interface, client_message, &client_message_size))
        {
            ZUC_WhitelistEntry_t whitelist[ZUC_SERVER_STATUS_MAX_WHITELIST_ENTRIES];
            ZUC_ServerStatusGetWhitelist(server->admin_area.status_handle, whitelist);
            for(int i = 0; i < ZUC_SERVER_STATUS_MAX_WHITELIST_ENTRIES; ++i)
            {
                if(whitelist[i].code == 0 || whitelist[i].address.sin_addr.s_addr == 0)
                {
                    continue;
                }

                ZUC_UdpSendTo(&server->upd_server_socket, &whitelist[i].address, client_message, client_message_size);
            }
        }
        else
        {
            break;
        }
    }
}

__attribute__((cold))
static void ZUC_HandleAdminEvent(ZUC_Server_t *server, ZUC_EventHandlingResult_t event)
{
    if(SIGUSR1 == event.identifier)
    {
        while(1)
        {
            ZUC_IpcQueueResult_t command = ZUC_PopFromIpcQueue(server->command_queue);
            if(command.result >= 0)
            {
                ZUC_HandleAdminQueueItem(&ZUC_server.admin_area, command.item);
            }
            else
            {
                break;
            }
            ZUC_FreeIpcQueueItem(&command.item);
        }
    }
}

__attribute__((hot))
static int32_t ZUC_EventLoopHandleEvent(ZUC_Server_t *server, ZUC_EventHandlingResult_t event)
{
    switch(event.trigger)
    {
        case ZUC_EVENT_TRIGGER_READ:
            if(event.identifier == server->upd_server_socket.filedescriptor)
            {
                //
                // User Stuff...
                //
                while(0 == ZUC_UdpRecvFrom(&server->upd_server_socket, &ZUC_server.udp_datagram_buffer))
                {
                    if(0 == ZUC_HandleUdpDatagram(&ZUC_server.admin_area, &ZUC_server.udp_datagram_buffer))
                    {
                        const int32_t result = ZUC_ClientInterfaceSend(
                            server->client_interface,
                            ZUC_server.udp_datagram_buffer.data + sizeof(uint64_t),
                            ZUC_server.udp_datagram_buffer.size - sizeof(uint64_t)
                        );
                        if(result != 0)
                        {
                            ZUC_LoggerError(ZUC_server.logger, "Unable to send UDP Datagram to local Client.\n");
                        }
                    }
                }
            }
            break;
        case ZUC_EVENT_TRIGGER_SIGNAL:
            ZUC_HandleAdminEvent(&ZUC_server, event);
            break;
         default:
            break; 
    }
    return 0;
}
