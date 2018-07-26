#pragma once

#include "ddclconfig.h"
#ifdef DDSYS_WIN
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#pragma comment(lib, "ws2_32.lib")

#define DDSOCK_FD            SOCKET
#define DDSOCK_ERROR         SOCKET_ERROR
#define DDSOCK_INVALID       INVALID_SOCKET
#define DDSOCK_NO_ERROR      NO_ERROR

#else
#include <sys/socket.h>
#define DDSOCK_FD            int
#define DDSOCK_ERROR         -1
#define DDSOCK_INVALID       -1
#define DDSOCK_NO_ERROR      0

#endif


#ifdef DDSYS_WIN
    #ifndef DDSOCKETPOLL_USE_SELECT
        #define DDSOCKETPOLL_USE_SELECT
        //#define DDSOCKETPOLL_USE_IOCP
    #endif
#else
    #ifndef DDSOCKETPOLL_USE_SELECT
        #ifdef DDSYS_LINUX
            #define DDSOCKETPOLL_USE_EPOLL
        #endif
        #ifdef DDSYS_APPLE
            #define DDSOCKETPOLL_USE_KQUEUE
        #endif
    #endif
#endif


#ifdef DDSOCKETPOLL_USE_SELECT
#ifndef DDSYS_WIN
#include <sys/select.h>
#endif

#define DDSOCKETPOLL_READ   (1)
#define DDSOCKETPOLL_WRITE  (1 << 1)
#define DDSOCKETPOLL_ERROR  (1 << 2)
#define DDSOCKETPOLL_BE_ERROR (1 << 3)
#endif


#ifdef DDSOCKETPOLL_USE_EPOLL
#include <sys/epoll.h>

#define DDSOCKETPOLL_READ   EPOLLIN
#define DDSOCKETPOLL_WRITE  EPOLLOUT
#define DDSOCKETPOLL_ERROR  (EPOLLERR | EPOLLHUP)
#endif


#ifdef DDSOCKETPOLL_USE_KQUEUE
#include <sys/event.h>

#define DDSOCKETPOLL_READ   EVFILT_READ
#define DDSOCKETPOLL_WRITE  EVFILT_WRITE
#define DDSOCKETPOLL_ERROR  0
#endif

typedef struct tag_ddcl_SocketEvent{
    DDSOCK_FD fd;
    void * ud;
    int evt;
    struct tag_ddcl_SocketEvent * next;
}ddcl_SocketEvent;


typedef struct tag_ddclSocketPoll ddcl_SocketPoll;


DDCLAPI ddcl_SocketPoll *
ddcl_new_socket_poll ();

DDCLAPI void
ddcl_free_socket_poll (ddcl_SocketPoll * poll);

DDCLAPI int
ddcl_add_in_socket_poll (ddcl_SocketPoll * poll, DDSOCK_FD fd, int e, void * ud);

DDCLAPI int
ddcl_del_in_socket_poll (ddcl_SocketPoll * poll, DDSOCK_FD fd);

DDCLAPI int
ddcl_set_evt_in_socket_poll (ddcl_SocketPoll * poll, DDSOCK_FD fd, int evt, void * ud);

DDCLAPI int
ddcl_wait_socket_poll (ddcl_SocketPoll * poll, ddcl_SocketEvent * evts, int max, int timeout);

DDCLAPI dduint32
ddcl_get_socket_poll_count(ddcl_SocketPoll * poll);


#ifdef DDSOCKETPOLL_USE_IOCP
#endif
