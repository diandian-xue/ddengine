#define DDCL_CORE

#include "ddclservice.h"
#include "ddclthread.h"
#include "ddcltimer.h"
#include "ddclmalloc.h"
#include "ddclmap.h"
#include "ddclerr.h"
#include "ddcllog.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>


#define DEFAULT_QUEUE_SIZE      64
#define DDCLSERVICE_WAITMS      1000
#define MSG_DATA_BUFF      0


typedef struct tag_MsgQueue{
    struct tag_Service * service;
}MsgQueue;

typedef struct tag_Service{
    ddcl_SpinLock lock;
    dduint32 cap;
    dduint32 head;
    dduint32 tail;
    dduint32 queue_count;
    struct tag_Service * next;
    int in_global; // 是否在全局队列里面
    int global; // 是否是工作线程调度的队列
    int exit; //是否已经退出
    ddcl_Msg * queue;
    ddcl_Service handle;
    void * ud;
    dduint32 session_idx;
    ddcl_MsgCB cb;
    ddcl_ExitServiceCB exit_cb;
}Service;

typedef struct tag_GlobalQueue{
    ddcl_SpinLock lock;
    Service * head;
    Service * tail;
}GlobalQueue;

static GlobalQueue _Q = { 0 };

static struct tag_HandleStorage{
    ddcl_Storage * hs;
    ddcl_RWLock lock;
} _H = { 0 };

static struct tag_Monitor{
    ddcl_Map * t_map;
    ddcl_SpinLock t_lock;
    ddcl_Cond cond;
} _M;

static void
_push_global_queue (Service * svr){
    assert(svr->next == NULL);
    ddcl_lock_spin(&(_Q.lock));
    if(_Q.tail) {
        _Q.tail->next = svr;
        _Q.tail = svr;
    } else {
        _Q.head = _Q.tail = svr;
    }
    ddcl_unlock_spin(&(_Q.lock));
}

static Service *
_pop_global_queue (){
    ddcl_lock_spin(&(_Q.lock));
    Service * svr = _Q.head;
    if(svr){
        _Q.head = svr->next;
        if(!_Q.head){
            assert(svr == _Q.tail);
            _Q.tail = NULL;
        }
        svr->next = NULL;
    }
    ddcl_unlock_spin(&(_Q.lock));
    return svr;
}

static ddcl_Service
_register_handle (Service ** s){
    ddcl_Service h;
    ddcl_wlock_rw(&(_H.lock));
    h = ddcl_register_in_storage(_H.hs, (void **)s);
    ddcl_wunlock_rw(&(_H.lock));
    return h;
}

static Service *
_grep_handle (ddcl_Service h){
    Service * s;
    ddcl_rlock_rw(&(_H.lock));
    s = ddcl_find_in_storage(_H.hs, h);
    ddcl_runlock_rw(&(_H.lock));
    return s;
}

static void
_delete_handle (ddcl_Service h){
    ddcl_wlock_rw(&(_H.lock));
    ddcl_del_in_storage(_H.hs, h);
    ddcl_wunlock_rw(&(_H.lock));
}

static Service *
_new_service (Service * svr, int global) {
    ddcl_init_spin(&(svr->lock));
    svr->cap = DEFAULT_QUEUE_SIZE;
    svr->queue = ddcl_malloc(sizeof(ddcl_Msg) * svr->cap);
    svr->next = NULL;
    svr->session_idx = 0;
    svr->global = global;
    svr->queue_count = 0;
    return svr;
}

static void
_expand_service_queue (Service * svr, dduint32 new_cap){
    ddcl_Msg * new_queue = ddcl_malloc(sizeof(ddcl_Msg) * new_cap);
    for (unsigned i = 0; i < svr->cap; i++) {
        new_queue[i] = svr->queue[(svr->head + i) % svr->cap];
    }
    svr->head = 0;
    svr->tail = svr->cap;
    svr->cap = new_cap;

    ddcl_free(svr->queue);
    svr->queue = new_queue;
}

static void
_push_service_queue (Service * svr, ddcl_Service self, int ptype, 
    int cmd, ddcl_Session session, const char * data, size_t sz, int free){

    ddcl_lock_spin(&(svr->lock));
    svr->queue_count++;
    ddcl_Msg * msg = &(svr->queue[svr->tail]);
    msg->ptype = ptype;
    msg->ud = svr->ud;
    msg->from = self;
    msg->self = svr->handle;
    msg->session = session;
    msg->data = data;
    msg->sz = sz;
    msg->cmd = cmd;
    msg->free = free;
   
    if (++svr->tail >= svr->cap) {
        svr->tail = 0;
    }
    if (svr->head == svr->tail) {
        _expand_service_queue(svr, svr->cap * 2);
    }
    if (svr->global && !svr->in_global) {
        svr->in_global = 1;
        _push_global_queue(svr);
    }

    ddcl_unlock_spin(&(svr->lock));
}

static int
_pop_service_queue (Service * svr, ddcl_Msg * msg, int max) {
    int ret = 0;
    ddcl_lock_spin(&(svr->lock));
    while (svr->head != svr->tail && ret < max) {
        msg[ret] = svr->queue[svr->head++];
        svr->queue_count --;
        ret += 1;
        int head = svr->head;
        int tail = svr->tail;
        int cap = svr->cap;

        if (head >= cap) {
            svr->head = head = 0;
        }
    }
    if(svr->queue_count == 0 && svr->cap > (DEFAULT_QUEUE_SIZE * 2)){
        svr->cap = DEFAULT_QUEUE_SIZE;
        ddcl_free(svr->queue);
        svr->queue = ddcl_malloc(sizeof(ddcl_Msg) * svr->cap);
        svr->head = svr->tail = 0;
    }
    ddcl_unlock_spin(&svr->lock);
    return ret;
}

static void
_free_service_queue (Service * svr){
    ddcl_log(svr->handle, "exit svr %ld", svr->handle);
    ddcl_destroy_spin(&(svr->lock));
    ddcl_free(svr->queue);
    if(svr->exit_cb){
        svr->exit_cb(svr->handle, svr->ud);
    }
    _delete_handle(svr->handle);
}

void
_dispatch_msg (Service * svr, ddcl_Msg * msg){
    svr->cb(msg);
    if(msg->data && msg->free)
        ddcl_free((void *)msg->data);
}

int
_dispatch_global_queue (){
    Service * svr = _pop_global_queue();
    if (!svr)
        return 0;
    ddcl_Msg msg[8];
    int ret = _pop_service_queue(svr, msg, 8);
    if(!ret){
        return 0;
    }
    for (int i = 0; i < ret; i ++){
        if (svr->exit) {
            break;
        }
        _dispatch_msg(svr, &(msg[i]));
    }

    if (svr->exit) {
        _free_service_queue(svr);
    }else {
        ddcl_lock_spin(&(svr->lock));
        if (svr->tail != svr->head) {
            svr->in_global = 1;
        }else {
            svr->in_global = 0;
        }
        if (svr->in_global) {
            _push_global_queue(svr);
        }
        ddcl_unlock_spin(&(svr->lock));
    }
    return 1;
}

static ddcl_Session
_new_session (Service * svr){
    ddcl_Session id = ++ svr->session_idx;
    if(id == 0){
        svr->session_idx ++;
        return 1;
    }
    return id;
}

static void *
_service_thread_fn (void * arg){
    MsgQueue * q = NULL;
    int non_count = 0;
    for(;;){
        while(!_dispatch_global_queue()){
            ddcl_wait_time_cond(&(_M.cond), DDCLSERVICE_WAITMS);
        }
    }
    return NULL;
}

DDCLAPI int
ddcl_init_service_module (ddcl * conf){
    memset(&_Q, 0, sizeof(GlobalQueue));
    ddcl_init_spin(&(_Q.lock));

    _H.hs = ddcl_new_storage(sizeof(Service), 0xFF + 1);
    ddcl_init_rw(&(_H.lock));

    _M.t_map = ddcl_new_map(NULL, NULL);
    ddcl_init_spin(&(_M.t_lock));
    ddcl_init_cond(&(_M.cond));

    for (dduint32 i = 0; i < conf->worker; i ++){
        ddcl_Thread t;
        ddcl_new_thread(&t, _service_thread_fn, NULL, 0);
        ddcl_set_map(_M.t_map, &t, sizeof(t), &t, sizeof(t), NULL);
    }
    return 0;
}

DDCLAPI int
ddcl_exit_service (ddcl_Service h){
    Service * svr = _grep_handle(h);
    if (svr){
        ddcl_lock_spin(&(svr->lock));
        svr->exit = 1;
        ddcl_unlock_spin(&(svr->lock));
        return 0;
    }else{
        return DDCL_SERVICE_UNKNOW_HANDLE;
    }
}

DDCLAPI int
ddcl_exit_service_cb(ddcl_Service h, ddcl_ExitServiceCB cb){
    Service * svr = _grep_handle(h);
    if (svr){
        svr->exit_cb = cb;
        return 0;
    }else{
        return DDCL_SERVICE_UNKNOW_HANDLE;
    }
}

DDCLAPI ddcl_Service
ddcl_new_service (ddcl_MsgCB cb, void * ud){
    Service * svr;
    ddcl_Service handle = _register_handle(&svr); 
    _new_service(svr, 1);
    svr->cb = cb;
    svr->ud = ud;
    svr->handle = handle;
    return handle;
}

DDCLAPI ddcl_Service
ddcl_new_service_not_worker (ddcl_MsgCB cb, void * ud) {
    Service * svr;
    ddcl_Service handle = _register_handle(&svr);
    _new_service(svr, 0);
    svr->cb = cb;
    svr->ud = ud;
    svr->handle = handle;
    return handle;
}

DDCLAPI int
ddcl_send (ddcl_Service to, ddcl_Service self, int ptype,
    int cmd, ddcl_Session session, const void * data, size_t sz){
    Service * tos = _grep_handle(to);
    if(!tos){
        return DDCL_SERVICE_UNKNOW_HANDLE;
    }
    _push_service_queue(tos, self, ptype, cmd, session, data, sz, 1);
    return 0;
}

DDCLAPI int
ddcl_send_b (ddcl_Service to, ddcl_Service self, int ptype, 
    int cmd, ddcl_Session session, const void * data, size_t sz){
    Service * tos = _grep_handle(to);
    if(!tos){
        return DDCL_SERVICE_UNKNOW_HANDLE;
    }
    char * buf = NULL;
    if (data){
        buf = malloc(sz);
        memcpy(buf, data, sz);
    }
    _push_service_queue(tos, self, ptype, cmd, session, buf, sz, 1);
    return 0;
}

DDCLAPI int
ddcl_send_raw (ddcl_Service to, ddcl_Service self, int ptype, 
    int cmd, ddcl_Session session, const void * data, size_t sz){
    Service * tos = _grep_handle(to);
    if(!tos){
        return DDCL_SERVICE_UNKNOW_HANDLE;
    }
    _push_service_queue(tos, self, ptype, cmd, session, data, sz, 0);
    return 0;
}

DDCLAPI int
ddcl_call (ddcl_Service to, ddcl_Service self, int ptype,
    int cmd, ddcl_Session * session, const void * data, size_t sz){
    Service * tos = _grep_handle(to); Service * selfs = _grep_handle(self);
    if (!tos || !selfs) {
        return DDCL_SERVICE_UNKNOW_HANDLE;
    }
    *session = _new_session (selfs);
    _push_service_queue(tos, self, ptype, cmd, *session, data, sz, 1);
    return 0;
}

DDCLAPI int
ddcl_call_b (ddcl_Service to, ddcl_Service self, int ptype,
    int cmd, ddcl_Session * session, const void * data, size_t sz){
    Service * tos = _grep_handle(to);
    Service * selfs = _grep_handle(self);
    if(!tos || !selfs){
        return DDCL_SERVICE_UNKNOW_HANDLE;
    }
    *session = _new_session (selfs);
    char * buf = NULL;
    if (data){
        buf = malloc(sz);
        memcpy(buf, data, sz);
    }
    _push_service_queue(tos, self, ptype, cmd, *session, buf, sz, 1);
    return 0;
}

DDCLAPI int
ddcl_call_raw (ddcl_Service to, ddcl_Service self, int ptype,
    int cmd, ddcl_Session * session, const void * data, size_t sz){
    Service * tos = _grep_handle(to);
    Service * selfs = _grep_handle(self);
    if (!tos || !selfs) {
        return DDCL_SERVICE_UNKNOW_HANDLE;
    }
    *session = _new_session (selfs);
    _push_service_queue(tos, self, ptype, cmd, *session, data, sz, 0);
    return 0;
}

DDCLAPI int
ddcl_set_service_ud(ddcl_Service h, void * ud){
    Service * svr = _grep_handle(h);
    if(svr){
        svr->ud = ud;
        return 0;
    }else{
        return DDCL_SERVICE_UNKNOW_HANDLE;
    }
}

DDCLAPI void *
ddcl_get_service_ud(ddcl_Service h){
    Service * svr = _grep_handle(h);
    if(svr){
        return svr->ud;
    }else{
        return NULL;
    }
}

DDCLAPI int
ddcl_start (ddcl_Service h){
    Service * svr = _grep_handle(h);
    if (!svr)
        return DDCL_SERVICE_UNKNOW_HANDLE;
    if(svr->in_global)
        return DDCL_SERVICE_UNKNOW_HANDLE;

    ddcl_Thread t = ddcl_self_thread();
    ddcl_lock_spin(&(_M.t_lock));
    if (ddcl_get_map(_M.t_map, &t, sizeof(t), NULL)){
        ddcl_unlock_spin(&(_M.t_lock));
        return DDCL_SERVICE_THREAD_IS_REGISTERED;
    }
    ddcl_set_map(_M.t_map, &t, sizeof(t), &t, sizeof(t), NULL);
    ddcl_unlock_spin(&(_M.t_lock));

    ddcl_Msg msg[8];
    for(;;){
        int ret = _pop_service_queue(svr, msg, 8);
        while (ret) {
            for (int i = 0; i < ret; i++){
                if(svr->exit){
                    ddcl_log(h, "exit service %ld", h);
                    return 0;
                }
                _dispatch_msg(svr, &(msg[i]));
            }
            ret = _pop_service_queue(svr, msg, 8);
        }
        ddcl_wait_time_cond(&(_M.cond), DDCLSERVICE_WAITMS);
    }
    return 0;
}

DDCLAPI int
ddcl_dispatch (ddcl_Service h){
    Service * svr = _grep_handle(h);
    if(!svr){
        return DDCL_SERVICE_UNKNOW_HANDLE;
    }
    ddcl_Msg msg;
    while(_pop_service_queue(svr, &msg, 1)){
        _dispatch_msg(svr, &msg);
    }
    return 0;
}

DDCLAPI int
ddcl_timeout (ddcl_Service h, ddcl_Session * session, dduint32 ms){
    Service * svr = _grep_handle(h);
    if(!svr){
        return DDCL_SERVICE_UNKNOW_HANDLE;
    }
    ddcl_Session sn = _new_session(svr);
    if(session)
        *session = sn;
    ddcl_add_timeout(h, sn, ms);
    return 0;
}

DDCLAPI void
ddcl_signal_monitor(){
    ddcl_signal_cond(&(_M.cond));
}

DDCLAPI int
ddcl_new_session(ddcl_Service h, ddcl_Session * session){
    Service * svr = _grep_handle(h);
    if(!svr){
        return DDCL_SERVICE_UNKNOW_HANDLE;
    }
    *session = _new_session(svr);
    return 0;
}
