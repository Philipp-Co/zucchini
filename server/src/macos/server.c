
#include <stdio.h>
#include <stdatomic.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <err.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <zucchini/server.h>
#include <zucchini/io/networking/upd.h>
#include <zucchini/io/networking/tcp.h>
#include <zucchini/admin/admin.h>
#include <zucchini/definitions.h>
#include <zucchini/ipc/queue.h>


#define ZUC_TCP_SERVER_PORT 9998
#define ZUC_UDP_SERVER_PORT 9999


typedef struct 
{
    int filedescriptor;
} ZUC_KQeue_t;

typedef struct 
{
    ZUC_KQeue_t server_socket_kqueue;
    ZUC_UdpServerSocket_t upd_server_socket;
    ZUC_UdpDatagram_t udp_datagram_buffer;

    ZUC_IpcQueue_t command_queue;
    //ZUC_TcpSocket_t admin_socket;
    //ZUC_TcpServerSocket_t tcp_server_socket;
    //ZUC_TcpMessage_t tcp_message_buffer;
    
    ZUC_Admin_t admin_area; 

    atomic_int terminate;

    uint8_t udp_message_buffer[1024];
} ZUC_Server_t;


static ZUC_Server_t ZUC_server = {0};


static int32_t ZUC_ObserverFiledescriptor(ZUC_Server_t *server, int filedescriptor, int trigger);
void ZUC_RemoveFromKQueue(ZUC_KQeue_t *queue, int filedescriptor);


static void ZUC_EventLoop(ZUC_Server_t *server);
static int32_t ZUC_EventLoopHandleEvent(ZUC_Server_t *server, struct kevent *event);

//static void ZUC_ReceiveFromTcpSocket(ZUC_TcpSocket_t *socket, ZUC_TcpMessage_t *message_buffer);
//static void ZUC_TcpConnectionOpened(ZUC_Server_t *server, ZUC_TcpSocket_t *socket);
//static void ZUC_TcpConnectionClosed(ZUC_Server_t *server);


__attribute__((cold))
int32_t ZUC_CreateServer(void)
{
    atomic_store(&ZUC_server.terminate, 0);

    int kqueue_instance = kqueue();
    if(kqueue_instance == -1)
    {
        printf("Unable to create kqueue instance\n");
        return -1;
    }
    ZUC_server.server_socket_kqueue.filedescriptor = kqueue_instance;

    ZUC_server.upd_server_socket = ZUC_CreateUdpServerSocket(ZUC_UDP_SERVER_PORT); 
    if(-1 == ZUC_server.upd_server_socket.filedescriptor)
    {
        return -1;
    }
    ZUC_ObserverFiledescriptor(&ZUC_server, ZUC_server.upd_server_socket.filedescriptor, EVFILT_READ);
    ZUC_server.udp_datagram_buffer.capacity = sizeof(ZUC_server.udp_message_buffer);
    ZUC_server.udp_datagram_buffer.size = 0;
    ZUC_server.udp_datagram_buffer.data = ZUC_server.udp_message_buffer;


    //ZUC_server.admin_socket.filedescriptor = -1;
    ZUC_InitializeAdminAdrea(&ZUC_server.admin_area);
    //ZUC_server.tcp_server_socket = ZUC_CreateTcpServerSocket(ZUC_TCP_SERVER_PORT);
    //if(-1 == ZUC_server.tcp_server_socket.filedescriptor)
    //{
    //   return -1;
    //} 
    //ZUC_ObserverFiledescriptor(&ZUC_server, ZUC_server.tcp_server_socket.filedescriptor, EVFILT_READ);

    ZUC_server.command_queue = ZUC_CreateIpcQueue();
    if(!ZUC_server.command_queue)
    {
        printf("Unable to create Ipc Queue.\n");
        return -1;
    }
    ZUC_ObserverFiledescriptor(&ZUC_server, ZUC_IpcQueueFiledescriptor(ZUC_server.command_queue), EVFILT_READ);
    //ZUC_server.tcp_message_buffer.data = malloc(4096);
    //ZUC_server.tcp_message_buffer.size = 0;
    //ZUC_server.tcp_message_buffer.capacity = 4096;

    return 0;
}

__attribute__((cold))
void ZUC_DestroyServer(void)
{
    close(ZUC_server.server_socket_kqueue.filedescriptor);
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

__attribute__((cold))
static int32_t ZUC_ObserverFiledescriptor(ZUC_Server_t *server, int filedescriptor, int trigger)
{
    struct kevent event;
    EV_SET(&event, filedescriptor, trigger, EV_ADD | EV_CLEAR, 0, 0, NULL);
    if(kevent(server->server_socket_kqueue.filedescriptor, &event, 1, NULL, 0, NULL) == -1)
    {
        printf("Unable to attach for kqeueu Event...\n");
        return -1;
    } 
    return 0;
}

#define ZUCCHINI_EVENTLOOP_NUMBER_OF_EVENTS 32
__attribute__((hot))
static void ZUC_EventLoop(ZUC_Server_t *server)
{
    struct kevent events[ZUCCHINI_EVENTLOOP_NUMBER_OF_EVENTS];
    const int result = kevent(
        server->server_socket_kqueue.filedescriptor, NULL, 0, events, ZUCCHINI_EVENTLOOP_NUMBER_OF_EVENTS, NULL 
    ); 
    for(int i=0;i<result;++i)
    {
        ZUC_EventLoopHandleEvent(&ZUC_server, &events[i]);
    }
}

__attribute__((hot))
static int32_t ZUC_EventLoopHandleEvent(ZUC_Server_t *server, struct kevent *event)
{
    if(ZUC_UNLIKELY(event->flags & (EV_ERROR | EV_EOF)))
    {
        //
        // Clean Up...
        //
        //const int fd = (int)event->ident;
        //if(fd == ZUC_server.admin_socket.filedescriptor)
        //{
        //    ZUC_TcpConnectionClosed(&ZUC_server);
        //}
    }
    else 
    {
        const int fd = (int)event->ident;
        if(fd == server->upd_server_socket.filedescriptor)
        {
            //
            // User Stuff...
            //
            if(0 == ZUC_UdpRecvFrom(&server->upd_server_socket, &ZUC_server.udp_datagram_buffer))
            {
                printf("Datagramm: %s\n", ZUC_server.udp_datagram_buffer.data);
            }
        }
        else if(fd == ZUC_IpcQueueFiledescriptor(server->command_queue))
        {
            ZUC_IpcQueueResult_t result = ZUC_PopFromIpcQueue(server->command_queue);
            if(0 == result.result)
            {
                printf("Request: %s\n", result.item.data);
            }
        }
        //else if(fd == ZUC_server.tcp_server_socket.filedescriptor)
        //{
            //
            // Admin stuff...
            //
        //    ZUC_TcpSocket_t socket = ZUC_TcpAccept(&ZUC_server.tcp_server_socket);
        //    if(-1 != socket.filedescriptor)
        //    {

        //        if(-1 == ZUC_server.admin_socket.filedescriptor)
        //        {
        //            ZUC_TcpConnectionOpened(&ZUC_server, &socket);
        //        }
        //        else
        //        {
        //            ZUC_DestroyTcpSocket(&socket);
        //        }
        //    }
        //    else
        //    {
        //        ZUC_DestroyTcpSocket(&socket);
        //    }
        //}
        //else if(fd == ZUC_server.admin_socket.filedescriptor)
        //{
        //    ZUC_ReceiveFromTcpSocket(&ZUC_server.admin_socket, &ZUC_server.tcp_message_buffer);
        //}
    }
    return 0;
}

//__attribute__((cold))
//static void ZUC_ReceiveFromTcpSocket(ZUC_TcpSocket_t *socket, ZUC_TcpMessage_t *message_buffer)
//{
//    if(0 == ZUC_TcpReceive(socket, message_buffer))
//    {
//        ZUC_OnAdminMessage(*message_buffer);
//    }
//}

//__attribute__((cold))
//static void ZUC_TcpConnectionOpened(ZUC_Server_t *server, ZUC_TcpSocket_t *socket)
//{
    //server->admin_socket = *socket;
    //ZUC_ObserverFiledescriptor(&ZUC_server, socket->filedescriptor, EVFILT_READ);
    //ZUC_ReceiveFromTcpSocket(&ZUC_server.admin_socket, &server->tcp_message_buffer);
//}

//__attribute__((cold))
//static void ZUC_TcpConnectionClosed(ZUC_Server_t *server)
//{
    //ZUC_RemoveFromKQueue(&server->server_socket_kqueue, server->admin_socket.filedescriptor);
    //ZUC_DestroyTcpSocket(&server->admin_socket);
    //server->admin_socket.filedescriptor = -1;
//}

__attribute__((cold))
void ZUC_RemoveFromKQueue(ZUC_KQeue_t *queue, int filedescriptor)
{
    struct kevent event;
    EV_SET(&event, filedescriptor, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    if(kevent(queue->filedescriptor, &event, 1, NULL, 0, NULL) == -1)
    {
    }
}
