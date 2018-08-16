
#include <stdio.h>
#include <memory.h>
#include "ddcl.h"
#include "ddclservice.h"
#include "ddclsocket.h"
#include "ddcllog.h"
#include "ddclmalloc.h"


static void
timeout_fn(ddcl_Msg * msg){
	ddcl_log(msg->self, "all poll socket count: %d", ddcl_getall_socket_count());

    ddcl_Session session;
    ddcl_timeout(msg->self, &session, 2000);
}

static void
msgcb(ddcl_Msg * msg){
    if (msg->cmd == DDCL_CMD_TIMEOUT){
        timeout_fn(msg);
        return;
    }

    ddcl_SocketRsp * rsp = (ddcl_SocketRsp *)msg->data;
    switch(rsp->cmd){
    case DDCL_SOCKET_ACCEPT:{
            //ddcl_log(msg->self, "on socket accept:%d %d", rsp->fd, ddcl_getall_socket_count());
            ddcl_Session session;
            ddcl_read_socket(rsp->fd, msg->self, 0, &session);
            break;
        }
    case DDCL_SOCKET_READ:{
            char * data = (char *)msg->data + sizeof(ddcl_SocketRsp);
            char * data_buf = ddcl_malloc(rsp->sz + 1);
            memcpy(data_buf, data, rsp->sz);
            data_buf[rsp->sz] = 0;
            //ddcl_log(msg->self, "on socket read: %d %s", rsp->fd, data_buf);
			ddcl_free(data_buf);

            char * content = "<!DOCTYPE html> <html> <head> <title>Welcome to ddengine-simple-httpd!</title>" \
    "<style> body { width: 35em; margin: 0 auto; font-family: Tahoma, Verdana, Arial, sans-serif;}</style></head>"\
    "<body><h1>hello world</h1></body>\n";
            char head[65535];
            sprintf(head, "HTTP/1.0 200 OK\r\nContent-length: %zu\r\n\r\n", strlen(content));
            ddcl_send_socket(rsp->fd, msg->self, head, strlen(head));
            ddcl_send_socket(rsp->fd, msg->self, content, strlen(content));
            ddcl_close_socket(rsp->fd, msg->self);

            ddcl_Session session;
            ddcl_read_socket(rsp->fd, msg->self, 0, &session);
            break;
        case DDCL_SOCKET_ERROR:
            //ddcl_log(msg->self, "socket close: %d", rsp->fd);
            break;
        }
    }
}

static void
on_socket_accept(ddcl_Msg * msg){
    printf("on_socket_accept:");
}

int main(int argc, char * argv[]){
    int port = 8000;

    ddcl conf;
    ddcl_default(&conf);
    conf.worker = 4;
    int ret = ddcl_init(&conf);

    if(ret){
        printf("ddcl_init failed %d\n", ret);
        return 1;
    }
    ddcl_Handle svr = ddcl_new_service_not_worker(msgcb, NULL);

    ddcl_Session session;
    ddcl_Handle listen = ddcl_listen_socket(svr, "0.0.0.0", port, 1000, 0);

    printf("start: %d %d  %d\n", svr, listen, port);
    fflush(stdout);

    ddcl_timeout(svr, &session, 2000);
    ddcl_start(svr);
    return 0;
}
