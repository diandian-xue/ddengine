#pragma once

#include "ddclconfig.h"

#ifdef DDSYS_WIN
#include <Windows.h>
#else
	#define _XOPEN_SOURCE 500
    #include <pthread.h>
    #include <unistd.h>

    #ifndef pthread_spinlock_t
        #define pthread_spinlock_t      pthread_mutex_t
        #define pthread_spin_init(l)    pthread_mutex_init(l, NULL)
        #define pthread_spin_lock(l)    pthread_mutex_lock(l)
        #define pthread_spin_trylock(l) pthread_mutex_trylock(l)
        #define pthread_spin_unlock(l)  pthread_mutex_unlock(l)
        #define pthread_spin_destroy(l) pthread_mutex_destroy(l)
    #endif
#endif


// 线程句柄
#ifdef DDSYS_WIN
typedef DWORD ddcl_Thread;
#else
typedef pthread_t ddcl_Thread;
#endif

// 自旋锁
typedef struct tag_ddcl_SpinLock {
#ifdef DDSYS_WIN
    BOOL lock;
#else
    pthread_spinlock_t  lock;
#endif
}ddcl_SpinLock;

// 可重用互斥锁
typedef struct tag_ddcl_Mutex {
#ifdef DDSYS_WIN
    CRITICAL_SECTION lock;
#else
    pthread_mutex_t lock;
#endif
}ddcl_Mutex;

// 信号量
typedef struct tag_ddcl_Cond {
#ifdef DDSYS_WIN
    HWND evt;
    BOOL lock;
#else
    pthread_mutex_t mutex;
    pthread_cond_t cond;
#endif
}ddcl_Cond;

//读写锁
typedef struct tag_ddcl_RWLock {
#ifdef DDSYS_WIN
    SRWLOCK lock;
    //CRITICAL_SECTION lock;
#else
    pthread_rwlock_t lock;
#endif
}ddcl_RWLock;

DDCLAPI int
ddcl_new_thread (ddcl_Thread * t, void *(*start_fn)(void *), void * arg, int detach);

DDCLAPI void
ddcl_exit_thread ();

DDCLAPI int
ddcl_cancel_thread (ddcl_Thread t);

DDCLAPI int
ddcl_join_thread (ddcl_Thread t);

DDCLAPI ddcl_Thread
ddcl_self_thread ();

DDCLAPI void
ddcl_sleepms (dduint32 ms);

DDCLAPI void
ddcl_init_spin(ddcl_SpinLock * l);

DDCLAPI int
ddcl_lock_spin(ddcl_SpinLock * l);

DDCLAPI int
ddcl_try_lock_spin(ddcl_SpinLock * l);

DDCLAPI int
ddcl_unlock_spin(ddcl_SpinLock * l);

DDCLAPI void
ddcl_destroy_spin(ddcl_SpinLock * l);



DDCLAPI void
ddcl_init_mutex(ddcl_Mutex * l);

DDCLAPI int
ddcl_lock_mutex(ddcl_Mutex * l);

DDCLAPI int
ddcl_unlock_mutex(ddcl_Mutex * l);

DDCLAPI void
ddcl_destroy_mutex(ddcl_Mutex * l);


DDCLAPI void
ddcl_init_cond (ddcl_Cond * cond);

DDCLAPI void
ddcl_destroy_cond(ddcl_Cond * cond);

DDCLAPI int
ddcl_wait_cond (ddcl_Cond * cond);

DDCLAPI int
ddcl_wait_time_cond (ddcl_Cond * cond, dduint32 ms);

DDCLAPI int
ddcl_signal_cond (ddcl_Cond * cond);


DDCLAPI void
ddcl_init_rw (ddcl_RWLock * l);

DDCLAPI void
ddcl_destroy_rw(ddcl_RWLock * l);

DDCLAPI void
ddcl_rlock_rw (ddcl_RWLock * l);

DDCLAPI void
ddcl_wlock_rw (ddcl_RWLock * l);

DDCLAPI void
ddcl_runlock_rw (ddcl_RWLock * l);

DDCLAPI void
ddcl_wunlock_rw (ddcl_RWLock * l);

