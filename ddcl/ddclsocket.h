#pragma once

#include "ddcl.h"
#include "ddclsocketpoll.h"
#include "ddclservice.h"

enum{
    DDCL_SOCKET_ACCEPT = 0,
    DDCL_SOCKET_READ,
    DDCL_SOCKET_ERROR,
};

typedef ddcl_Handle ddcl_Socket;

typedef struct tag_ddcl_SocketRsp{
    ddcl_Socket fd;
    int cmd;
    size_t sz;
}ddcl_SocketRsp;

DDCLAPI int
ddcl_socket_module_init (ddcl * conf);

DDCLAPI ddcl_Socket
ddcl_connect_socket (ddcl_Service from, const char * host, dduint16 port, ddcl_Session session);

DDCLAPI ddcl_Socket
ddcl_listen_socket (ddcl_Service from, const char * host, dduint16 port, int backlog, ddcl_Session session);

DDCLAPI int
ddcl_accept_socket (ddcl_Socket fd, ddcl_Service from, ddcl_Session * session);

DDCLAPI int
ddcl_read_socket(ddcl_Socket fd, ddcl_Service from, size_t sz, ddcl_Session * session);

DDCLAPI int
ddcl_send_socket(ddcl_Socket fd, ddcl_Service from, const void * buf, size_t sz);

DDCLAPI int
ddcl_close_socket(ddcl_Socket fd, ddcl_Service from);

DDCLAPI dduint32
ddcl_getall_socket_count();

DDCLAPI int
ddcl_forward_socket (ddcl_Socket fd, ddcl_Service from);
