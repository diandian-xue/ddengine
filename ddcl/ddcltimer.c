#define DDCL_CORE

#include "ddcltimer.h"
#include "ddclthread.h"

#ifdef DDSYS_WIN
#include <Windows.h>
#else
#include <sys/time.h>
#endif

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

static struct {
#ifdef DDSYS_WIN
    ddcl_SpinLock lock;
    LARGE_INTEGER counter;
    LARGE_INTEGER frequency;
    dduint64 startup;
#endif
} _T;

#define TIME_NEAR_SHIFT     8
#define TIME_NEAR           (1 << TIME_NEAR_SHIFT)
#define TIME_LEVEL_SHIFT    8
#define TIME_LEVEL          (1 << TIME_LEVEL_SHIFT)
#define TIME_NEAR_MASK      (TIME_NEAR - 1)
#define TIME_LEVEL_MASK     (TIME_LEVEL - 1)

static dduint32 _MS; 

typedef struct tag_Node{
    struct tag_Node * next;
    dduint64 expire;
    struct {
        ddcl_Handle source;
        ddcl_Session session;
    }e;
}Node;

typedef struct tag_List{
    Node head;
    Node * tail;
}List;

typedef struct tag_Timer{
    ddcl_SpinLock lock;
    List n[TIME_NEAR];
    List t[7][TIME_LEVEL];
    dduint64 time;
    dduint64 stime;
    dduint64 current;
    dduint64 current_point;
}Timer;

static Timer _TR;

static inline Node *
_clear_list (List * l){
    Node * r = l->head.next;
    l->head.next = NULL;
    l->tail = &(l->head);
    return r;
}

static inline void
_link_list (List * l, Node * n){
    l->tail->next = n;
    l->tail = n;
    n->next = NULL;
}

static void
_add_node (Node * n){
    dduint64 time = n->expire;
    if((time | TIME_NEAR_MASK) == (_TR.time | TIME_NEAR_MASK)){
        _link_list(&(_TR.n[time & TIME_NEAR_MASK]), n);
    }else{
        int i = 0;
        dduint64 mask = TIME_NEAR << TIME_LEVEL_SHIFT;
        for(i = 0; i < 13; i ++){
            if((time | (mask - 1)) == (_TR.time | (mask - 1))){
                break;
            }
            mask <<= TIME_LEVEL_SHIFT;
        }
        int shift = TIME_NEAR_SHIFT + i * TIME_LEVEL_SHIFT;
        _link_list(&(_TR.t[i][(time >> shift) & TIME_LEVEL_MASK]), n);
    }
}

static void
_move_list (int level, int idx){
    Node * n = _clear_list(&(_TR.t[level][idx]));
    while(n){
        Node * t = n->next;
        _add_node(n);
        n = t;
    }
}

static void
_shift_timer (){
    int mask = TIME_NEAR;
    dduint64 ct = ++ _TR.time;
    if(ct == 0){
        _move_list(3, 0);
    }else{
        dduint64 t = ct >> TIME_NEAR_SHIFT;
        int i = 0;
        while((ct & (mask - 1)) == 0){
            int idx = t & TIME_LEVEL_MASK;
            if(idx != 0){
                _move_list(i, idx);
                break;
            }
            mask <<= TIME_LEVEL_SHIFT;
            t >>= TIME_LEVEL_SHIFT;
            i ++;
        }
    }
}

static void
_dispatch (Node * n){
    do{
        ddcl_send(n->e.source, 0, DDCL_PTYPE_RESP,
                DDCL_CMD_TIMEOUT, n->e.session, NULL, 0);
        Node * t = n;
        n = n->next;
        free(t);
    } while (n);
}

static void
_execute_timer (){
    int idx = _TR.time & TIME_NEAR_MASK;
    while (_TR.n[idx].head.next) {
        Node * n = _clear_list(&(_TR.n[idx]));
        ddcl_unlock_spin(&(_TR.lock));
        // dispatch timeout msg
        _dispatch(n);
        ddcl_lock_spin(&(_TR.lock));
    }
}

static void
_update_timer (){
    ddcl_lock_spin(&(_TR.lock));
    _execute_timer();
    _shift_timer();
    _execute_timer();
    ddcl_unlock_spin(&(_TR.lock));
}

static void *
_timer_thread_fn (void * arg){
    for (;;) {
        dduint64 ct = ddcl_now();
        if (ct < _TR.current_point) {
            printf("time diff error: change from %lld to %lld \n", ct, _TR.current_point);
            _TR.current_point = ct;
        }else if(ct != _TR.current_point) {
            dduint32 diff = (dduint32)(ct - _TR.current_point);
            _TR.current_point = ct;
            _TR.current += diff;
            for (dduint32 i = 0; i < diff; i++) {
                _update_timer();
            }
        }else {
            ddcl_sleepms(_MS);
        }
        ddcl_signal_monitor();
    }
}

DDCLAPI int
ddcl_timer_module_init (ddcl * conf){
    _MS = conf->timer_ms;
    memset(&_T, 0, sizeof(_T));
#ifdef DDSYS_WIN
    ddcl_init_spin(&_T.lock);
    QueryPerformanceCounter(&_T.counter);
    QueryPerformanceFrequency(&_T.frequency);
    _T.startup = ddcl_systime();
#endif

    memset(&_TR, 0, sizeof(Timer));
    _TR.current_point = ddcl_now();
    for(int i = 0; i < TIME_NEAR; i ++){
        _clear_list(&(_TR.n[i]));
    }
    for(int i = 0; i < 7; i ++){
        for(int j = 0; j < TIME_LEVEL; j ++){
            _clear_list(&(_TR.t[i][j]));
        }
    }

    ddcl_Thread t;
    ddcl_new_thread(&t, _timer_thread_fn, NULL, 0);
    return 0;
}

DDCLAPI dduint64
ddcl_now (){

#ifdef DDSYS_WIN
    ddcl_lock_spin(&_T.lock);
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    double s = ((double)(li.QuadPart - _T.counter.QuadPart) / _T.frequency.QuadPart) * 1000;
    ddcl_unlock_spin(&_T.lock);
    return _T.startup + (dduint64)s;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

DDCLAPI dduint64
ddcl_systime(){
#ifdef DDSYS_WIN
    time_t t = time(NULL);
    SYSTEMTIME st;
    GetSystemTime(&st);
    return t * 1000 + st.wMilliseconds;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

DDCLAPI void
ddcl_add_timeout (ddcl_Handle h, ddcl_Session session, dduint32 ms){
    if (ms > 0) {
        Node * n = malloc(sizeof(Node));
        n->e.session = session;
        n->e.source = h;
        ddcl_lock_spin(&(_TR.lock));
        n->expire = _TR.time + ms - 1;
        _add_node(n);
        ddcl_unlock_spin(&(_TR.lock));
    }else {
        ddcl_send(h, 0, DDCL_PTYPE_RESP, DDCL_CMD_TIMEOUT, session, NULL, 0);
    }
}
