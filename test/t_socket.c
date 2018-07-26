#include "ddclsocket.h"
#include "ddclservice.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

static ddcl_Handle g_svr;
static ddcl_Handle g_sock;
static char * str;
static size_t len;

/*
static void
_on_socket_msg_resp(ddcl_Msg * msg){
	ddcl_SocketRsp * rsp = (ddcl_SocketRsp *)msg->data.u.b.p;
}

static void
_on_socket_recv(ddcl_Msg * msg){
	ddcl_SocketRsp * rsp = (ddcl_SocketRsp *)msg->data.u.b.p;
	int cmd = msg->data.cmd;
	ddcl_Handle h = rsp->h;

	if(cmd == DDCLSOCKET_OK){
		int sz = rsp->sz;
		char * buf = malloc(sz + 1);
		buf[sz] = 0;
		memcpy(buf, rsp->data, sz);

		ddcl_Session sn;
		ddcl_new_session(g_svr, _on_socket_recv, NULL, &sn);
		sn.data.u.h = h;
		int ret = ddcl_read_socket(h, 0, &sn);

		char * content = buf;
		char * content = "<!DOCTYPE html> <html> <head> <title>Welcome to nginx!</title>" \
"<style> body { width: 35em; margin: 0 auto; font-family: Tahoma, Verdana, Arial, sans-serif;}</style></head>"\
"<body><h1>hello</h1></body>";
		char head[65535];
		sprintf(head, "HTTP/1.0 200 OK\r\nContent-length: %d\r\n\r\n", strlen(content));
		ddcl_send_socket(h, head, strlen(head));
		ddcl_send_socket(h, content, strlen(content));
		free(buf);

		ddcl_close_socket(h);
	}
	else {
		ddcl_close_socket(h);
	}
}

static void
on_socket_accept(ddcl_Msg * msg){
	ddcl_SocketRsp * rsp = (ddcl_SocketRsp *)msg->data.u.b.p;
	ddcl_Handle h = rsp->h;

	ddcl_Session sn;
	ddcl_new_session(g_svr, _on_socket_recv, NULL, &sn);
	sn.data.u.h = h;
	ddcl_read_socket(h, 0, &sn);
}

void t_socket(ddcl_Handle svr){
	g_svr = svr;
	printf("ddclsocket test start ----------\n\n");

	ddcl_Session session;
	ddcl_new_session(svr, _on_socket_msg_resp, NULL, &session);
	//ddcl_Handle h = ddcl_SocketConnect("127.0.0.1", 80, &session);
	//printf("ddcl_SocketConnect:%d\n", h);

	ddcl_new_session(svr, on_socket_accept, NULL, &session);
	ddcl_Handle listen = ddcl_listen_socket("0.0.0.0", 8001, 1000, &session);
	printf("ddcl_listen_socket: %d\n", listen);

	printf("\nddclsocket test ended ----------\n\n");

	FILE * f = fopen("C:\\Users\\diandian\\Desktop\\xx.html", "r");
	str = malloc(6553500);
	len = fread(str, 1, 6553500, f);
	fclose(f);
}

**/
