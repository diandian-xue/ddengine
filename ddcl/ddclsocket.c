#define DDCL_CORE

#include "ddclsocket.h"
#include "ddclthread.h"
#include "ddclmalloc.h"
#include "ddclerr.h"
#include "ddcllog.h"

#ifdef DDSYS_WIN

#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <sys/types.h>
    #include <arpa/inet.h>
    #include <sys/ioctl.h>
    #include <sys/select.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include <assert.h>

#define MAX_POLL_WAIT  64
#define STORAGE_INIT_SIZE 512
#define DEFAULT_RECV_SIZE 512

enum {
    _SS_CONNECTING = 1,
    _SS_CONNECTED,
    _SS_LISTENING,
    _SS_CLOSE,
};

enum {
    _SCMD_CONNECT = 1,
    _SCMD_LISTEN,
    _SCMD_ACCEPT,
    _SCMD_SEND,
    _SCMD_READ,
    _SCMD_CLOSE,
    _SCMD_FORWARD,
};

enum {
    PROTOCOL_TCP = 0,
    PROTOCOL_UDP = 1,
    PROTOCOL_UDPv6  = 2,
};

typedef struct tag_SocketCmd{
    int cmd;
    ddcl_Socket h;
    DDSOCK_FD fd;
    size_t sz;
    char * data;
}SocketCmd;

typedef struct tag_ReadBuff{
    size_t sz;
    ddcl_Service source;
    ddcl_Session session;
    struct tag_ReadBuff * next;
}ReadBuff;

typedef struct tag_SendBuff{
    size_t sz;
    char * buf;
    struct tag_SendBuff * next;
}SendBuff;

typedef struct tag_CacheBuff{
    char * buf;
    size_t sz;
    struct tag_CacheBuff * next;
}CacheBuff;

typedef struct tag_SocketThread{
    ddcl_SocketPoll * poll;
    ddcl_Thread thread;
    ddcl_Service svr;
}SocketThread;

typedef struct tag_Socket{
    DDSOCK_FD fd;
    ddcl_Socket h;
    int protocol;
    int status;
    int forward;
    int poll_evt;
    ddcl_Service source;
    ddcl_Session session;
    SocketThread * st;

    char * crbuf;
    CacheBuff * cbuf;
    CacheBuff * cbuf_end;
    size_t cbuf_sz;

    ReadBuff * rbuf;
    ReadBuff * rbuf_end;

    SendBuff * sbuf;
    SendBuff * sbuf_end;
} Socket;

typedef union tag_SockAddrAll{
    struct sockaddr s;
    struct sockaddr_in v4;
    struct sockaddr_in6 v6;
}SockAddrAll;

static struct{
    ddcl_Storage * hs;
    ddcl_RWLock lock;
} _H = { 0 };

static SocketThread * _T;
static dduint32 _thread_count = 0;

static void
_rsp_socket(SocketThread * st, Socket * s,
        ddcl_Service source, ddcl_Session session, int cmd){
    ddcl_SocketRsp rsp;
    rsp.fd = s->h;
    rsp.cmd = cmd;
    rsp.sz = 0;
    ddcl_send_b(source, st->svr, session ? DDCL_PTYPE_RESP : DDCL_PTYPE_SEND,
        DDCL_CMD_SOCKET, session, (char *)&rsp, sizeof(rsp));
}

static void
_rsp_read(ddcl_Service self, ddcl_Socket h, ddcl_Service source, 
    ddcl_Session session, int cmd, char * data, size_t sz){
    ddcl_SocketRsp rsp;
    rsp.fd = h;
    rsp.cmd = cmd;
    rsp.sz = sz;
    char * buf = (char *)&rsp;
    if (sz > 0){
        buf = ddcl_malloc(sizeof(rsp) + sz);
        memcpy(buf, &rsp, sizeof(rsp));
        memcpy(buf + sizeof(rsp), data, sz);
    }
    ddcl_send_b(source, self, session ? DDCL_PTYPE_RESP : DDCL_PTYPE_SEND,
        DDCL_CMD_SOCKET, session, buf, sizeof(rsp) + sz);
    if(sz > 0)
        ddcl_free(buf);
}

static char *
_read_cache_buff(Socket * s, size_t sz){
    s->cbuf_sz -= sz;
    CacheBuff * cb = s->cbuf;
    CacheBuff * tmp;
    char * buf = ddcl_malloc(sz);
    size_t cur = 0;
    size_t copysz = 0;
    while(cb && sz){
        copysz = (sz <= cb->sz) ? sz : cb->sz;
        memcpy(buf + cur, cb->buf, copysz);
        sz -= copysz;
        cb->sz -= copysz;
        if (cb->sz){
            memcpy(cb->buf, cb->buf + copysz, cb->sz);
        }else{
            tmp = cb;
            cb = cb->next;

            if (s->cbuf == tmp) {
                s->cbuf = cb;
            }
            if(s->cbuf_end == tmp)
                s->cbuf_end = cb;
            ddcl_free(tmp->buf);
            ddcl_free(tmp);
        }

    }
    return buf;
}

static void
_free_cache_buff(CacheBuff * cb){
    CacheBuff * tmp;
    while(cb){
        if(cb->buf)
            ddcl_free(cb->buf);
        tmp = cb;
        cb = cb->next;
        ddcl_free(tmp);
    }
}

static void
_push_cache_buff(Socket * s, char * buf, size_t sz, int bcopy){
    CacheBuff * cb  = ddcl_malloc(sizeof(CacheBuff));
    if (bcopy){
        cb->buf = ddcl_malloc(sz);
        memcpy(cb->buf, buf, sz);
    }else{
        cb->buf = buf;
    }
    cb->sz = sz;
    cb->next = NULL;
    s->cbuf_sz += sz;
    if (s->cbuf_end){
        s->cbuf_end->next = cb;
        s->cbuf_end = cb;
    }else{

        s->cbuf = cb;
        s->cbuf_end = cb;
    }
}

static void
_free_read_buff(ReadBuff * rb){
    ReadBuff * tmp;
    while(rb){
        tmp = rb;
        rb = rb->next;
        ddcl_free(tmp);
    }
}

static int
_read_buff_is_empty(Socket * s){
    return s->rbuf == NULL;
}

static void
_push_read_buff (Socket * s, size_t sz, ddcl_Service source, ddcl_Session session){
    ReadBuff * rb = ddcl_malloc(sizeof(ReadBuff));
    memset(rb, 0, sizeof(ReadBuff));
    rb->sz = sz;
    rb->session = session;
    rb->source = source;
    if(s->rbuf_end){
        s->rbuf_end->next = rb;
        s->rbuf_end = rb;
    }else{
        s->rbuf = rb;
        s->rbuf_end = rb;
    }
}

static ReadBuff *
_pop_read_buff(Socket * s){
    ReadBuff * rb = s->rbuf;
    if(rb){
        if(rb == s->rbuf_end)
            s->rbuf_end = s->rbuf_end->next;
        s->rbuf = s->rbuf->next;
        return rb->next;
    }
    return NULL;
}

static int
_send_buff_is_empty(Socket * s){
    return s->sbuf == NULL;
}

static void
_push_send_buff(Socket * s, char * buf, size_t sz){
    SendBuff * sb = ddcl_malloc(sizeof(SendBuff));
    memset(sb, 0, sizeof(SendBuff));
    sb->sz = sz;
    sb->buf = ddcl_malloc(sz);
    memcpy(sb->buf, buf, sz);

    if(s->sbuf_end){
        s->sbuf_end->next = sb;
        s->sbuf_end = sb;
    }else{
        s->sbuf = sb;
        s->sbuf_end = sb;
    }
}

static SendBuff *
_pop_send_buff(Socket * s){
    SendBuff * sb = s->sbuf;
    if(sb){
        if(sb == s->sbuf_end)
            s->sbuf_end = s->sbuf_end->next;
        s->sbuf = s->sbuf->next;
        return sb->next;
    }
    return NULL;
}

static void
_set_keepalive (DDSOCK_FD fd) {
    int keepalive = 1;
    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive, sizeof(keepalive));
}

static void
_set_non_blocking (DDSOCK_FD fd) {
#ifdef DDSYS_WIN
    u_long flag = 1;
    ioctlsocket(fd, FIONBIO, &flag);
#else
    int flag = fcntl(fd, F_GETFL, 0);
    if (-1 == flag) {
        return;
    }
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
#endif
}

static inline SocketThread *
_st_find(ddcl_Socket h){
    return &(_T[h % _thread_count]);
}

static void
_close_fd (DDSOCK_FD fd) {
#ifdef DDSYS_WIN
    closesocket(fd);
#else
    close(fd);
#endif
}

static void
_add_in_poll(ddcl_Socket h, Socket * s, int evt){
    unsigned hash = h % _thread_count;
    SocketThread * st = &(_T[hash]);
    s->st = st;
    s->poll_evt = evt;
    ddcl_add_in_socket_poll(st->poll, s->fd, evt, ((char *)0) + h);
}

static void
_add_evt_2_poll(ddcl_SocketPoll * poll, Socket * s, int evt){
    int e = s->poll_evt | evt;
    if(s->poll_evt == e)
        return;
    s->poll_evt = e;
    ddcl_set_evt_in_socket_poll(poll, s->fd, e, ((char *)0) + s->h);
}

static void
_del_evt_in_poll(ddcl_SocketPoll * poll, Socket * s, int evt){
    int e = s->poll_evt & (~evt);
    if(s->poll_evt == e)
        return;
    s->poll_evt = e;
    ddcl_set_evt_in_socket_poll(poll, s->fd, e, ((char *)0) + s->h);
}

static Socket *
_register_fd(DDSOCK_FD fd, int status){
    ddcl_Socket h;
    Socket * s;
    ddcl_wlock_rw(&(_H.lock));
    h = ddcl_register_in_storage(_H.hs, (void **)&s);
    ddcl_wunlock_rw(&(_H.lock));
    s->fd = fd;
    s->h = h;
    s->status = status;
    s->crbuf = ddcl_malloc(DEFAULT_RECV_SIZE);
    return s;
}

static void
_del_fd (Socket * s){
    _free_cache_buff(s->cbuf);
    ddcl_free(s->crbuf);

    ddcl_del_in_socket_poll(s->st->poll, s->fd);
    _close_fd(s->fd);

    if(s->forward){
        _rsp_socket(s->st, s, s->source, 0, DDCL_SOCKET_ERROR);
    }
    ReadBuff * tmp;
    ReadBuff * rb = s->rbuf;
    while (rb) {
        tmp = rb;
        _rsp_read(s->st->svr, s->h, rb->source,
            rb->session, DDCL_SOCKET_ERROR, NULL, 0);
        rb = rb->next;
        ddcl_free(tmp);
    }
    ddcl_wlock_rw(&(_H.lock));
    ddcl_del_in_storage(_H.hs, s->h);
    ddcl_wunlock_rw(&(_H.lock));
}

static Socket *
_find_socket(ddcl_Socket h){
    ddcl_rlock_rw(&(_H.lock));
    Socket * s = (Socket *)ddcl_find_in_storage(_H.hs, h);
    if(!s || s->h != h){
        ddcl_runlock_rw(&(_H.lock));
        return NULL;
    }
    ddcl_runlock_rw(&(_H.lock));
    return s;
}

static void
_execute_event_in_fd_connecting(SocketThread * st, Socket * s, int evt){
    if(evt & DDSOCKETPOLL_WRITE){
#if defined(DDSYS_LINUX) && defined(DDSOCKETPOLL_USE_SELECT)
        int error;
        socklen_t len = sizeof (error);
        getsockopt(s->fd, SOL_SOCKET, SO_ERROR, &error, &len);
        if (error) {
            evt = DDSOCKETPOLL_ERROR;
        }else
#endif
        {
            s->status = _SS_CONNECTED;
            _add_evt_2_poll(st->poll, s, DDSOCKETPOLL_ERROR);
            _del_evt_in_poll(st->poll, s, DDSOCKETPOLL_WRITE);
            _rsp_socket(st, s, s->source, s->session, DDCL_SOCKET_READ);
        }
    }
    if(evt & DDSOCKETPOLL_ERROR){
        _del_fd(s);
    }
}

static void
_connected_event_by_forward (SocketThread * st, Socket * s, int evt){
    if(evt & DDSOCKETPOLL_READ){
        int rsz;
        for(;;){
            rsz = recv(s->fd, s->crbuf, DEFAULT_RECV_SIZE, 0);
            if(rsz > 0){
                _rsp_read(st->svr, s->h, s->source, 0,
                        DDCL_SOCKET_READ, s->crbuf, rsz);
                if(rsz < DEFAULT_RECV_SIZE){
                    break;
                }
            }else{

#ifdef DDSYS_WIN
                if (WSAGetLastError() != WSAEWOULDBLOCK) {
                    _del_fd(s);
                    s = NULL;
                }
#else
                int e = errno;
                if (e != EINTR || e != EWOULDBLOCK) {
                    _del_fd(s);
                    s = NULL;
                }
#endif
                break;
            }
        }
    }
    if(!s){
        return;
    }
    if(evt & DDSOCKETPOLL_ERROR){
        _del_fd(s);
    }
}

static void
_execute_event_on_fd_connected(SocketThread * st, Socket * s, int evt) {
    if (s->forward){
        _connected_event_by_forward(st, s, evt);
        return;
    }
    if(evt & DDSOCKETPOLL_READ){

        ReadBuff * rb = s->rbuf;
        if(_read_buff_is_empty(s) && s->forward){
            _del_evt_in_poll(st->poll, s, DDSOCKETPOLL_READ);
            return;
        }

        size_t rlen;
        char * rbuf;
        int rsz;
        while(rb){
            if(s->cbuf_sz > 0 && s->cbuf_sz >= rb->sz){
                rbuf = _read_cache_buff(s, rb->sz);
                _rsp_read(st->svr, s->h, rb->source, rb->session,
                    DDCL_SOCKET_READ, rbuf, rb->sz);
                ddcl_free(rbuf);
                rb = _pop_read_buff(s);
                continue;
            }

            if (rb->sz == 0)
                rlen = DEFAULT_RECV_SIZE;
            else
                rlen = rb->sz - s->cbuf_sz;
            if (rlen <= DEFAULT_RECV_SIZE)
                rbuf = s->crbuf;
            else
                rbuf = ddcl_malloc(rlen);
            rsz = recv(s->fd, rbuf, (int)rlen, 0);
            if(rsz > 0){
                _push_cache_buff(s, rbuf, rsz, rlen <= DEFAULT_RECV_SIZE);
                rb->sz = rsz;
                continue;
            }
#ifdef DDSYS_WIN
            if (WSAGetLastError() != WSAEWOULDBLOCK) {
                _del_fd(s);
                s = NULL;
            }
#else
            int e = errno;
            if (e != EINTR || e != EWOULDBLOCK) {
                _del_fd(s);
                s = NULL;
            }
#endif
            if (rsz > DEFAULT_RECV_SIZE)
                ddcl_free(rbuf);
            break;
        }

        if (s) {
            if (!rb)
                _del_evt_in_poll(st->poll, s, DDSOCKETPOLL_READ);
        }else{
            return;
        }
    }

    if(evt & DDSOCKETPOLL_WRITE){
        SendBuff * sb = s->sbuf;
        size_t slen = 0;
        int n;
        while(sb){
            n = send(s->fd, sb->buf + slen, (int)(sb->sz - slen), 0);
            if(n <= 0){
                sb->buf += slen;
                sb->sz -= slen;
                break;
            }
            slen += n;
            if (slen >= sb->sz){
                slen = 0;
                sb = _pop_send_buff(s);
                continue;
            }
        }
        if (!sb)
            _del_evt_in_poll(st->poll, s, DDSOCKETPOLL_WRITE);
    }

    if(evt & DDSOCKETPOLL_ERROR){
        _del_fd(s);
    }
}

static void
_listening_event_by_forward (SocketThread * st, Socket * s, int evt){
    if(evt & DDSOCKETPOLL_READ){
        DDSOCK_FD nfd;
        Socket * ns;
        SockAddrAll saa;
        socklen_t len = sizeof(SockAddrAll);

        for(;;){
            nfd = accept(s->fd, &(saa.s), &len);
            if(nfd == -1){
                return;
            }

            _set_keepalive(nfd);
            _set_non_blocking(nfd);

            ns = _register_fd(nfd, _SS_CONNECTED);
            ns->source = s->source;
            ns->session = s->session;
            _add_in_poll(ns->h, ns, DDSOCKETPOLL_ERROR);
            ns->poll_evt = DDSOCKETPOLL_ERROR;
            _rsp_socket(st, ns, s->source, 0, DDCL_SOCKET_ACCEPT);
        }
    }
}

static void
_execute_event_on_fd_listening(SocketThread * st, Socket * s, int evt){
    if(evt & DDSOCKETPOLL_ERROR){
        _del_fd(s);
        return;
    }
    if(s->forward){
        _listening_event_by_forward(st, s, evt);
        return;
    }
    if(evt & DDSOCKETPOLL_READ){
        if(_read_buff_is_empty(s)){
            _del_evt_in_poll(st->poll, s, DDSOCKETPOLL_READ);
            return;
        }

        DDSOCK_FD nfd;
        Socket * ns;
        SockAddrAll saa;
        socklen_t len = sizeof(SockAddrAll);
        ReadBuff * rb = s->rbuf;
        while(rb){
            nfd = accept(s->fd, &(saa.s), &len);
            if(nfd == -1){
                return;
            }

            _set_keepalive(nfd);
            _set_non_blocking(nfd);

            ns = _register_fd(nfd, _SS_CONNECTED);
            ns->source = s->source;
            ns->session = s->session;
            _add_in_poll(ns->h, ns, DDSOCKETPOLL_ERROR);
            ns->poll_evt = DDSOCKETPOLL_ERROR;
            _rsp_socket(st, ns, rb->source, rb->session, DDCL_SOCKET_ACCEPT);

            rb = _pop_read_buff(s);
        }

        if(_read_buff_is_empty(s)){
            _del_evt_in_poll(st->poll, s, DDSOCKETPOLL_READ);
        }
    }
}

static void
_execute_event_in_fd_close(SocketThread * st, Socket * s, int evt){
    _del_fd(s);
}

static void
_execute_event(SocketThread * st, ddcl_Socket h, int evt){
    Socket * s = _find_socket(h);
    if(!s)
        return;
    switch(s->status){
    case _SS_CONNECTING:
        _execute_event_in_fd_connecting(st, s, evt);
        break;
    case _SS_CONNECTED:
        _execute_event_on_fd_connected(st, s, evt);
        break;
    case _SS_LISTENING:
        _execute_event_on_fd_listening(st, s, evt);
        break;
        /*
    case _SS_CLOSE:
        _execute_event_in_fd_close(st, s, evt);
        break;
        */
    default:
        assert(0);
        break;
    }
}

static void
_timeout_wait_poll(ddcl_Msg * msg){
    ddcl_timeout(msg->self, NULL, 5);
    SocketThread * st = msg->ud;
    ddcl_SocketEvent evts[MAX_POLL_WAIT] = { 0 };
    ddcl_SocketEvent * e;
    ddcl_Socket h;
    int n = 0;
    n = ddcl_wait_socket_poll(st->poll, evts, MAX_POLL_WAIT, 1);
    for (int i = 0; i < n; i++) {
        e = &(evts[i]);
        h = (ddcl_Socket)e->ud;
        _execute_event(st, h, e->evt);
    }
}

static void *
_socket_thread_fn (void * arg){
    SocketThread * st = (SocketThread *)arg;
    ddcl_Service svr = st->svr;
    ddcl_timeout(svr, NULL, 2);
    ddcl_start(svr);
    return NULL;
}

static void
_excute_read_cmd(SocketCmd * cmd, ddcl_Service source, ddcl_Session session){
    ddcl_Socket h = cmd->h;
    Socket * s = _find_socket(h);
    if(!s || s->fd != cmd->fd){
        const char * err = "socket fd error";
        ddcl_send_b(source, 0, DDCL_PTYPE_RESP,
            DDCL_CMD_ERROR, session, err, strlen(err) + 1);
        return;
    }
    if(s->forward){
        const char * err = "socket fd is forward mode";
        ddcl_send_b(source, 0, DDCL_PTYPE_RESP,
            DDCL_CMD_ERROR, session, err, strlen(err) + 1);
        return;
    }


    size_t sz = cmd->sz;
    if(!_read_buff_is_empty(s)){
        _push_read_buff(s, cmd->sz, source, session);
        _add_evt_2_poll(s->st->poll, s, DDSOCKETPOLL_READ);
        return;
    }

    size_t rlen = 0;
    char * rbuf;
    for(;;){
        if(s->cbuf_sz > 0 && s->cbuf_sz >= sz){
            if(sz == 0)
                sz = s->cbuf_sz;
            _rsp_read(s->st->svr, s->h, source, session,
                DDCL_SOCKET_READ, _read_cache_buff(s, sz), sz);
            return;
        }
        if (sz == 0)
            rlen = DEFAULT_RECV_SIZE;
        else
            rlen = sz - s->cbuf_sz;
        if(rlen <= DEFAULT_RECV_SIZE)
            rbuf = s->crbuf;
        else
            rbuf = ddcl_malloc(rlen);

        int rsz = recv(s->fd, rbuf, (int)rlen, 0);
        if(rsz > 0){
            _push_cache_buff(s, rbuf, rsz, rlen <= DEFAULT_RECV_SIZE);
            if(rlen > DEFAULT_RECV_SIZE)
                ddcl_free(rbuf);
        }else{
            if(rlen > DEFAULT_RECV_SIZE)
                ddcl_free(rbuf);
            break;
        }
    }

    _push_read_buff(s, sz, source, session);
    if(!(s->poll_evt & DDSOCKETPOLL_READ))
        _add_evt_2_poll(s->st->poll, s, DDSOCKETPOLL_READ);
}

static void
_excute_send_cmd(SocketCmd * cmd, ddcl_Service source, ddcl_Session session){
    ddcl_Socket h = cmd->h;
    Socket * s = _find_socket(h);
    if(!s || s->fd != cmd->fd){
        return;
    }
    char * buf = cmd->data;
    size_t sz = cmd->sz;
    size_t slen = 0;
    if(_send_buff_is_empty(s)){
        int n;
        for(;;){
            n = send(s->fd, buf + slen, (int)(sz - slen), 0);
            if(n <= 0)
                break;
            slen += n;
            if (slen >= sz) {
                ddcl_free(cmd->data);
                return;
            }
        }
    }

    _push_send_buff(s, buf + slen, sz - slen);
    _add_evt_2_poll(s->st->poll, s, DDSOCKETPOLL_WRITE);
    ddcl_free(cmd->data);
}

static void
_excute_close_cmd(SocketCmd * cmd, ddcl_Service source, ddcl_Session session){
    Socket * s = _find_socket(cmd->h);
    if(s && s->fd == cmd->fd){
        _del_fd(s);
    }
}

static void
_excute_accept_cmd(SocketCmd * cmd, ddcl_Service source, ddcl_Session session){
    ddcl_Socket h = cmd->h;
    Socket * s = _find_socket(cmd->h);
    if(!s || s->fd != cmd->fd){
        const char * err = "unknow listen fd for accepting";
        ddcl_send_b(source, 0, DDCL_PTYPE_RESP,
            DDCL_CMD_ERROR, session, err, strlen(err) + 1);
        return;
    }
    if(s->forward){
        const char * err = "socket fd is forward mode";
        ddcl_send_b(source, 0, DDCL_PTYPE_RESP,
            DDCL_CMD_ERROR, session, err, strlen(err) + 1);
        return;
    }

    if(!_read_buff_is_empty(s)){
        _push_read_buff(s, cmd->sz, source, session);
        _add_evt_2_poll(s->st->poll, s, DDSOCKETPOLL_READ | DDSOCKETPOLL_ERROR);
        return;
    }

    SockAddrAll saa;
    socklen_t len = sizeof(SockAddrAll);
    DDSOCK_FD fd = accept(s->fd, &(saa.s), &len);
    if(fd != -1){
        _set_keepalive(fd);
        _set_non_blocking(fd);
        Socket *ns = _register_fd(fd, _SS_CONNECTED);
        ns->source = s->source;
        ns->session = s->session;
        _add_in_poll(ns->h, ns, DDSOCKETPOLL_ERROR);
        ns->poll_evt = DDSOCKETPOLL_ERROR;
        _rsp_socket(ns->st, ns, source, session, DDCL_SOCKET_ACCEPT);
    }else{
        _push_read_buff(s, 0, source, session);
        if(!(s->poll_evt & DDSOCKETPOLL_READ))
            _add_evt_2_poll(s->st->poll, s, DDSOCKETPOLL_READ);
    }

}

static void
_excute_forward_cmd (SocketCmd * cmd, ddcl_Service source, ddcl_Session session){
    ddcl_Socket h = cmd->h;
    Socket * s = _find_socket(cmd->h);
    if(!s || s->fd != cmd->fd){
        return;
    }
    s->forward = 1;
    _add_evt_2_poll(s->st->poll, s, DDSOCKETPOLL_READ | DDSOCKETPOLL_ERROR);
}

static void
_socket_svr_msg_cb(ddcl_Msg * msg){
    switch(msg->cmd){
    case DDCL_CMD_SOCKET:{
        SocketCmd * cmd = (SocketCmd*)msg->data;
        switch(cmd->cmd){
            case _SCMD_READ:
                _excute_read_cmd(cmd, msg->from, msg->session);
                break;
            case _SCMD_SEND:
                _excute_send_cmd(cmd, msg->from, msg->session);
                break;
            case _SCMD_CLOSE:
                _excute_close_cmd(cmd, msg->from, msg->session);
                break;
            case _SCMD_LISTEN:
                break;
            case _SCMD_ACCEPT:
                _excute_accept_cmd(cmd, msg->from, msg->session);
                break;
            case _SCMD_FORWARD:
                _excute_forward_cmd(cmd, msg->from, msg->session);
                break;
        }
    };
    break;
    case DDCL_CMD_TIMEOUT:
        _timeout_wait_poll(msg);
        break;
    default:
        assert(0);
        break;
    }
    
}

DDCLAPI int
ddcl_socket_module_init (ddcl * conf){
    dduint32 thread = conf->socket;
    if(thread > 64)
        thread = 64;

#ifdef DDSYS_WIN
    WSADATA * lpwsaData = (WSADATA *)NULL;
    if (lpwsaData == NULL){
        WSADATA wsaData;
        lpwsaData = &wsaData;
    }
    WORD wVersionRequested = MAKEWORD(1, 1);
    int nResult = WSAStartup(wVersionRequested, lpwsaData);
    if (LOBYTE(lpwsaData->wVersion) != 1 || HIBYTE(lpwsaData->wVersion) != 1){
        WSACleanup();
        return 1;
    }
#endif

    _thread_count = thread;
    _H.hs = ddcl_new_storage(sizeof(Socket), STORAGE_INIT_SIZE);
    ddcl_init_rw(&(_H.lock));

    _T = ddcl_malloc(sizeof(SocketThread) * thread);
    memset(_T, 0, sizeof(SocketThread) * thread);
    for(unsigned i = 0; i < thread; i ++){
        SocketThread * st = &(_T[i]);
        st->poll = ddcl_new_socket_poll();
        st->svr = ddcl_new_service_not_worker(_socket_svr_msg_cb, st);
        ddcl_new_thread(&(st->thread), _socket_thread_fn, st, 0);
    }

    return 0;
}

DDCLAPI ddcl_Socket
ddcl_connect_socket (ddcl_Service from, const char * host, dduint16 port, ddcl_Session session){
    struct addrinfo ai_hints = {0};
    struct addrinfo *ai_list = NULL;
    struct addrinfo *ai_ptr = NULL; 
    ai_hints.ai_family = AF_UNSPEC;
    ai_hints.ai_socktype = SOCK_STREAM;
    ai_hints.ai_protocol = IPPROTO_TCP;

    int status;
    char ports[16];
    sprintf (ports, "%d", port);
    status = getaddrinfo(host, ports, &ai_hints, &ai_list);
    if (status) {
        ddcl_log(from, "getaddrinfo err: %s\n", gai_strerror(status));
        freeaddrinfo(ai_list);
        return 0;
    }

    DDSOCK_FD fd = DDSOCK_INVALID;
    for (ai_ptr = ai_list; ai_ptr != NULL; ai_ptr = ai_ptr->ai_next) {
        fd = socket(ai_ptr->ai_family, ai_ptr->ai_socktype, ai_ptr->ai_protocol);
        if (fd < 0) {
            fd = DDSOCK_INVALID;
            continue;
        }
        _set_keepalive(fd);
        _set_non_blocking(fd);
        status = connect(fd, ai_ptr->ai_addr, (int)ai_ptr->ai_addrlen);
        if (status != 0) {
#ifdef DDSYS_WIN
            if (WSAGetLastError() != WSAEWOULDBLOCK) {
                _close_fd(fd);
                fd = DDSOCK_INVALID;
                continue;
            }
#else
            int e = errno;
            if (e != EINPROGRESS) {
                _close_fd(fd);
                fd = DDSOCK_INVALID;
                continue;
            }
#endif
        }
        break;
    }

    freeaddrinfo(ai_list);
    if (fd == DDSOCK_INVALID) {
        return 0;
    }
    Socket * s = _register_fd(fd, _SS_CONNECTING);
    s->session = session;
    s->source = from;
    ddcl_Socket h = s->h;
    _add_in_poll(s->h, s, DDSOCKETPOLL_WRITE | DDSOCKETPOLL_ERROR);
    return h;
}


DDCLAPI ddcl_Socket
ddcl_listen_socket (ddcl_Service from, const char * host, dduint16 port, int backlog, ddcl_Session session){
    struct addrinfo ai_hints = {0};
    struct addrinfo *ai_list = NULL;
    struct addrinfo *ai_ptr = NULL; 
    ai_hints.ai_family = AF_UNSPEC;
    ai_hints.ai_socktype = SOCK_STREAM;
    ai_hints.ai_protocol = IPPROTO_TCP;

    int status;
    char ports[16];
    sprintf (ports, "%d", port);
    status = getaddrinfo(host, ports, &ai_hints, &ai_list);
    if (status) {
        printf("getaddrinfo err: %s\n", gai_strerror(status));
        freeaddrinfo(ai_list);
        return 0;
    }

    DDSOCK_FD fd = DDSOCK_INVALID;
    for (ai_ptr = ai_list; ai_ptr != NULL; ai_ptr = ai_ptr->ai_next) {
        fd = socket(ai_ptr->ai_family, ai_ptr->ai_socktype, ai_ptr->ai_protocol);
        if (fd < 0) {
            fd = DDSOCK_INVALID;
            continue;
        }
        //_set_keepalive(fd);
        _set_non_blocking(fd);
        status = bind(fd, ai_ptr->ai_addr, (int)ai_ptr->ai_addrlen);
        if (status != 0) {
#ifdef DDSYS_WIN
            if (WSAGetLastError() != WSAEWOULDBLOCK) {
                _close_fd(fd);
                fd = DDSOCK_INVALID;
                continue;
            }
#else
            int e = errno;
            if (e != EINPROGRESS) {
                _close_fd(fd);
                fd = DDSOCK_INVALID;
                continue;
            }
#endif
        }
        break;
    }

    freeaddrinfo(ai_list);
    if (fd == DDSOCK_INVALID) {
        return 0;
    }
    if (listen(fd, backlog) == -1) {
        _close_fd(fd);
        return 0;
    }
    Socket * s = _register_fd(fd, _SS_LISTENING);
    s->source = from;
    s->session = session;
    _add_in_poll(s->h, s, DDSOCKETPOLL_READ | DDSOCKETPOLL_ERROR);
    return s->h;
}

DDCLAPI int
ddcl_accept_socket (ddcl_Socket fd, ddcl_Service from, ddcl_Session * session){
    Socket * s = _find_socket(fd);
    if (!s)
        return DDCLSOCKET_INVALID_HANDLE;

    SocketCmd cmd;
    cmd.cmd = _SCMD_ACCEPT;
    cmd.h = fd;
    cmd.fd = s->fd;
    cmd.sz = 0;
    SocketThread * st = _st_find(fd);
    ddcl_call_b(st->svr, from, DDCL_PTYPE_SEND, 
        DDCL_CMD_SOCKET, session, (char *)&cmd, sizeof(cmd));

    return DDCL_OK;
}

DDCLAPI int
ddcl_read_socket(ddcl_Socket fd, ddcl_Service from, size_t sz, ddcl_Session * session){
    Socket * s = _find_socket(fd);
    if (!s)
        return DDCLSOCKET_INVALID_HANDLE;

    SocketCmd cmd;
    cmd.cmd = _SCMD_READ;
    cmd.h = fd;
    cmd.fd = s->fd;
    cmd.sz = sz;
    SocketThread * st = _st_find(fd);
    ddcl_call_b(st->svr, from, DDCL_PTYPE_SEND, 
        DDCL_CMD_SOCKET, session, (char *)&cmd, sizeof(cmd));

    return DDCL_OK;
}

DDCLAPI int
ddcl_send_socket(ddcl_Socket fd, ddcl_Service from, const void * buf, size_t sz){
    Socket * s = _find_socket(fd);
    if(!s)
        return DDCLSOCKET_INVALID_HANDLE;

    SocketCmd cmd;
    cmd.cmd = _SCMD_SEND;
    cmd.h = fd;
    cmd.fd = s->fd;
    cmd.sz = sz;
    cmd.data = ddcl_malloc(sz);
    memcpy(cmd.data, buf, sz);

    SocketThread * st = _st_find(fd);
    ddcl_send_b(st->svr, from, DDCL_PTYPE_SEND,
        DDCL_CMD_SOCKET, 0, (char *)&cmd, sizeof(SocketCmd));

    return DDCL_OK;
}

DDCLAPI int
ddcl_close_socket(ddcl_Socket fd, ddcl_Service from){
    Socket * s = _find_socket(fd);
    if (!s)
        return DDCLSOCKET_INVALID_HANDLE;

    SocketCmd cmd;
    cmd.cmd = _SCMD_CLOSE;
    cmd.h = fd;
    cmd.fd = s->fd;
    SocketThread * st = _st_find(fd);
    ddcl_send_b(st->svr, from, DDCL_PTYPE_SEND,
        DDCL_CMD_SOCKET, 0, (char *)&cmd, sizeof(SocketCmd));

    return DDCL_OK;
}

DDCLAPI dduint32
ddcl_getall_socket_count(){
    dduint32 count = 0;
    for (dduint32 i = 0; i < _thread_count; i ++){
        count += ddcl_get_socket_poll_count(_T[i].poll);
    }
    return count;
}

DDCLAPI int
ddcl_forward_socket (ddcl_Socket fd, ddcl_Service from){
    Socket * s = _find_socket(fd);
    if (!s)
        return DDCLSOCKET_INVALID_HANDLE;

    SocketCmd cmd;
    cmd.cmd = _SCMD_FORWARD;
    cmd.h = fd;
    cmd.fd = s->fd;
    SocketThread * st = _st_find(fd);
    ddcl_send_b(st->svr, from, DDCL_PTYPE_SEND,
        DDCL_CMD_SOCKET, 0, (char *)&cmd, sizeof(SocketCmd));
    return DDCL_OK;
}
