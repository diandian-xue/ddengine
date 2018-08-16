#define DDCL_CORE

#include "ddclsocketpoll.h"
#include "ddclstorage.h"
#include "ddclthread.h"
#include "ddcltimer.h"
#include "ddclmalloc.h"

#ifdef DDSOCKETPOLL_USE_EPOLL
#include <sys/epoll.h>
#endif

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>

#define DDCL_FD_SETSIZE  64


struct tag_ddclSocketPoll{
#ifdef DDSOCKETPOLL_USE_SELECT
    ddcl_SocketEvent * sets;
    ddcl_SocketEvent * end;
    ddcl_SocketEvent * cur;
    ddcl_Mutex lock;
#endif

#ifdef DDSOCKETPOLL_USE_IOCP
    HANDLE cp;
#endif

#ifdef DDSOCKETPOLL_USE_EPOLL
    DDSOCK_FD epoll_fd;
#endif

#ifdef DDSOCKETPOLL_USE_KQUEUE
    DDSOCK_FD kqueue_fd;
#endif
    dduint32 sets_sz;
};


DDCLAPI ddcl_SocketPoll *
ddcl_new_socket_poll () {
    ddcl_SocketPoll * poll = ddcl_malloc(sizeof(ddcl_SocketPoll));
    memset(poll, 0, sizeof(ddcl_SocketPoll));

#ifdef DDSOCKETPOLL_USE_SELECT
    ddcl_init_mutex(&(poll->lock));
#endif

#ifdef DDSOCKETPOLL_USE_IOCP
#endif

#ifdef DDSOCKETPOLL_USE_EPOLL
    poll->epoll_fd = epoll_create(1024);
    if (poll->epoll_fd == -1){
        ddcl_free(poll);
        return NULL;
    }
#endif

#ifdef DDSOCKETPOLL_USE_KQUEUE
    poll->kqueue_fd = kqueue();
#endif
    return poll;
}

DDCLAPI void
ddcl_free_socket_poll (ddcl_SocketPoll * poll) {
#ifdef DDSOCKETPOLL_USE_SELECT
    ddcl_lock_mutex(&(poll->lock));
    ddcl_SocketEvent * e = poll->sets;
    while(e){
        ddcl_SocketEvent * tmp = e;
        e = e->next;
        ddcl_free(tmp);
    }
    poll->sets = NULL;
    poll->end = NULL;
    ddcl_destroy_mutex(&(poll->lock));
#endif

#ifdef DDSOCKETPOLL_USE_EPOLL
    close(poll->epoll_fd);
#endif

#ifdef DDSOCKETPOLL_USE_KQUEUE
    close(poll->kqueue_fd);
#endif

    ddcl_free(poll);
}


DDCLAPI int
ddcl_add_in_socket_poll (ddcl_SocketPoll * poll, DDSOCK_FD fd, int evt, void * ud) {

#ifdef DDSOCKETPOLL_USE_SELECT
    ddcl_lock_mutex(&(poll->lock));
    ddcl_SocketEvent * e = poll->sets;
    while(e){
        if(e->fd == fd){
            e->evt = evt;
            e->ud = ud;
            ddcl_unlock_mutex(&(poll->lock));
            return 0;
        }
        e = e->next;
    }

    e = ddcl_malloc(sizeof(ddcl_SocketEvent));
    memset(e, 0, sizeof(ddcl_SocketEvent));
    e->fd = fd;
    e->evt = evt;
    e->ud = ud;
    if(poll->end){
        poll->end->next = e;
        poll->end = e;
    }else{
        poll->sets = e;
        poll->end = e;
    }
    poll->sets_sz ++;
    ddcl_unlock_mutex(&(poll->lock));
    return 0;
#endif

#ifdef DDSOCKETPOLL_USE_EPOLL
    poll->sets_sz ++;
    struct epoll_event ev;
    ev.events = evt;
    ev.data.ptr = ud;
    if (epoll_ctl(poll->epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        return 1;
    }
#endif

#ifdef DDSOCKETPOLL_USE_KQUEUE
    poll->sets_sz ++;
    struct kevent ke;
    EV_SET(&ke, fd, EVFILT_READ, EV_ADD, 0, 0, ud);
    if (kevent(poll->kqueue_fd, &ke, 1, NULL, 0, NULL) == -1 || ke.flags & EV_ERROR) {
        return 1;
    }
    EV_SET(&ke, fd, EVFILT_WRITE, EV_ADD, 0, 0, ud);
    if (kevent(poll->kqueue_fd, &ke, 1, NULL, 0, NULL) == -1 || ke.flags & EV_ERROR) {
        EV_SET(&ke, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        kevent(poll->kqueue_fd, &ke, 1, NULL, 0, NULL);
        return 1;
    }
    return ddcl_set_evt_in_socket_poll(poll, fd, evt, ud);
#endif

    return 0;
}

DDCLAPI int
ddcl_del_in_socket_poll(ddcl_SocketPoll * poll, DDSOCK_FD fd) {
#ifdef DDSOCKETPOLL_USE_SELECT
    ddcl_SocketEvent * pre = NULL;
    ddcl_lock_mutex(&(poll->lock));
    ddcl_SocketEvent * e = poll->sets;
    while(e){
        if(e->fd == fd){
            if (pre)
                pre->next = e->next;
            else
                poll->sets = e->next;
            if (poll->end == e)
                poll->end = pre;
            poll->sets_sz --;
            ddcl_unlock_mutex(&(poll->lock));
            ddcl_free(e);
            return 0;
        }
        pre = e;
        e = e->next;
    }
     ddcl_unlock_mutex(&(poll->lock));
     assert(0);
    return 1;
#endif

#ifdef DDSOCKETPOLL_USE_EPOLL
    poll->sets_sz --;
    return epoll_ctl(poll->epoll_fd, EPOLL_CTL_DEL, fd , NULL) == -1;
#endif


#ifdef DDSOCKETPOLL_USE_KQUEUE
    poll->sets_sz --;
    struct kevent ke;
    EV_SET(&ke, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    kevent(poll->kqueue_fd, &ke, 1, NULL, 0, NULL);
    EV_SET(&ke, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    kevent(poll->kqueue_fd, &ke, 1, NULL, 0, NULL);
#endif
    return 0;
}


DDCLAPI int
ddcl_set_evt_in_socket_poll(ddcl_SocketPoll * poll, DDSOCK_FD fd, int evt, void * ud) {
#ifdef DDSOCKETPOLL_USE_SELECT
    ddcl_lock_mutex(&(poll->lock));
    ddcl_SocketEvent * e = poll->sets;
    while(e){
        if(e->fd == fd){
            e->evt = evt;
            e->ud = ud;
            ddcl_unlock_mutex(&(poll->lock));
            return 0;
        }
        e = e->next;
    }
    ddcl_unlock_mutex(&(poll->lock));
    return 1;
#endif

#ifdef DDSOCKETPOLL_USE_EPOLL
    struct epoll_event ev;
    ev.events = evt;
    ev.data.ptr = ud;
    epoll_ctl(poll->epoll_fd, EPOLL_CTL_MOD, fd, &ev);
#endif

#ifdef DDSOCKETPOLL_USE_KQUEUE
    struct kevent ke;
    int opt = (evt & DDSOCKETPOLL_READ) ? EV_ENABLE : EV_DISABLE;
    EV_SET(&ke, fd, EVFILT_READ, opt, 0, 0, ud);
    kevent(poll->kqueue_fd, &ke, 1, NULL, 0, NULL);


    opt = (evt & DDSOCKETPOLL_WRITE) ? EV_ENABLE : EV_DISABLE;
    EV_SET(&ke, fd, EVFILT_WRITE, opt, 0, 0, ud);
    kevent(poll->kqueue_fd, &ke, 1, NULL, 0, NULL);
#endif

    return 0;
}

#ifdef DDSOCKETPOLL_USE_SELECT
static int 
_selectWait(ddcl_SocketEvent ** e, ddcl_SocketEvent * evts, int max){
    int n = 0;
    int fdsz = 0;
    int maxfd = 0;
    fd_set rd, wd, ed;
    FD_ZERO(&rd); FD_ZERO(&wd); FD_ZERO(&ed);
    ddcl_SocketEvent * cur = *e;
    while(cur){
        if(fdsz >= DDCL_FD_SETSIZE)
            break;
        fdsz ++;
        if (cur->evt & DDSOCKETPOLL_READ)
            FD_SET(cur->fd, &rd);
        if(cur->evt & DDSOCKETPOLL_WRITE)
            FD_SET(cur->fd, &wd);
        if (cur->evt & DDSOCKETPOLL_ERROR) {
            FD_SET(cur->fd, &ed);
            /*
            int error = 0; 
            int len = sizeof(int);
            int ret = getsockopt(cur->fd, SOL_SOCKET, SO_ERROR, &error, &len);
            if (ret == DDSOCK_INVALID || error) {
                cur->evt |= DDSOCKETPOLL_BE_ERROR;
                n += 1;
            }
             * */
        }
        if(cur->fd >= maxfd)
            maxfd = (int)cur->fd;
        cur = cur->next;
    }
    struct timeval wt = { 0 };
    //tv.tv_sec * 1000 + tv.tv_usec / 1000;
    //wt.tv_sec = 1;
    //wt.tv_usec = 1000;
    if (select(maxfd + 1, &rd, &wd, &ed, &wt) <= 0 && n == 0)
        return 0;

    ddcl_SocketEvent * begin = *e;
    *e = cur;
    ddcl_SocketEvent * result;
    for(int i = 0; i < fdsz; i ++){
        if(n >= max)
            return n;

        result = &(evts[n]);
        result->evt = 0;
        if((begin->evt & DDSOCKETPOLL_READ) && FD_ISSET(begin->fd, &rd))
            result->evt |= DDSOCKETPOLL_READ;
        if((begin->evt & DDSOCKETPOLL_WRITE) && FD_ISSET(begin->fd, &wd))
            result->evt |= DDSOCKETPOLL_WRITE;
        if(((begin->evt & DDSOCKETPOLL_ERROR) && FD_ISSET(begin->fd, &ed))
            || (begin->evt & DDSOCKETPOLL_BE_ERROR))
            result->evt |= DDSOCKETPOLL_ERROR;
        if (result->evt > 0){
            n ++;
            //result->fd = begin->fd;
            result->ud = begin->ud;
        }
        begin = begin->next;
    }
    return n;

}
#endif

DDCLAPI int
ddcl_wait_socket_poll(ddcl_SocketPoll * poll, ddcl_SocketEvent * evts, int max, int timeout) {
#ifdef DDSOCKETPOLL_USE_SELECT
    int n = 0;
    dduint64 now = ddcl_now();
    for(;;){
        ddcl_lock_mutex(&(poll->lock));
        ddcl_SocketEvent * e = poll->sets;
        while(e){
            n += _selectWait(&e, evts + n, max - n);
            if(n > 0){
                ddcl_unlock_mutex(&(poll->lock));
                return n;
            }
            e = e->next;
        }
        ddcl_unlock_mutex(&(poll->lock));
        if(timeout > 1){
            if((ddcl_now() - now) >= timeout)
                return n;
        }
        else if(timeout == 1) {
            break;
        }
        ddcl_sleepms(10);
    }
    return n;
#endif

#ifdef DDSOCKETPOLL_USE_EPOLL
    struct epoll_event evs[max];
    int n = epoll_wait(poll->epoll_fd, evs, max, timeout);
    if (n <= 0)
        return n;
    ddcl_SocketEvent * e;
    for(int i = 0; i < n; i ++){
        e = &(evts[i]);
        e->fd = 0;
        e->ud = evs[i].data.ptr;
        e->evt = evs[i].events;
    }
    return n;
#endif

#ifdef DDSOCKETPOLL_USE_KQUEUE
    struct kevent evs[max];
    struct timespec * tp = NULL;
    if(timeout>=0){
        struct timespec t = { timeout / 1000, timeout * 1000000};
        tp = &t;
    }
    int n = kevent(poll->kqueue_fd, NULL, 0, evs, max, tp);
    if(n <= 0)
        return n;
    ddcl_SocketEvent * e;
    int evt = 0;
    for(int i = 0; i < n; i ++){
        e = &(evts[i]);
        e->fd = 0;
        e->ud = evs[i].udata;
        if(evs[i].filter == -2){
            e->evt = DDSOCKETPOLL_WRITE;
        }else if(evs[i].filter == -1){
            e->evt = DDSOCKETPOLL_READ;
        }else{
            e->evt = 0;
        }
    }
    return n;
#endif

    return 0;
}

DDCLAPI dduint32
ddcl_get_socket_poll_count(ddcl_SocketPoll * poll){
    return poll->sets_sz;
}
