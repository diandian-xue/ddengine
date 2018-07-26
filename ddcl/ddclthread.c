#define DDCL_CORE

#include "ddclthread.h"
#include "ddcltimer.h"
#include <time.h>
#include <stdio.h>

#ifdef DDSYS_WIN
#else
#include <sys/time.h>
#endif

DDCLAPI int
ddcl_new_thread (ddcl_Thread * t, void *(*start_fn)(void *), void * arg, int detach){
#ifdef DDSYS_WIN
    SECURITY_ATTRIBUTES psa;
    psa.nLength = sizeof(sizeof(psa));
    psa.bInheritHandle = detach;
    psa.lpSecurityDescriptor = NULL;
    HANDLE h = CreateThread(&psa, 0, (LPTHREAD_START_ROUTINE) start_fn, arg, 0, t);
    if (h == NULL)
        return -1;
    CloseHandle(h);
    return 0;
#else
    return pthread_create(t, NULL, start_fn, arg);
#endif
}

DDCLAPI void
ddcl_exit_thread (){
#ifdef DDSYS_WIN
    ExitThread(0);
#else
    pthread_exit(NULL);
#endif
}

DDCLAPI int
ddcl_cancel_thread (ddcl_Thread t) {
#ifdef DDSYS_WIN
    HANDLE h = OpenThread(THREAD_ALL_ACCESS, FALSE, t);
    BOOL r =  TerminateThread(h, 0);
    CloseHandle(h);
    return r;
#else
    return pthread_cancel(t);
#endif
}

DDCLAPI int
ddcl_join_thread (ddcl_Thread t) {
#ifdef DDSYS_WIN
    HANDLE h = OpenThread(THREAD_ALL_ACCESS, FALSE, t);
    DWORD r = WaitForSingleObject(h, INFINITE);
    CloseHandle(h);
    return r;
#else
    return pthread_join(t, NULL);
#endif
}

DDCLAPI ddcl_Thread
ddcl_self_thread (){
#ifdef DDSYS_WIN
    return GetCurrentThreadId();
#else
    return pthread_self();
#endif
}

DDCLAPI void
ddcl_sleepms (dduint32 ms){
#ifdef DDSYS_WIN
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}



DDCLAPI void
ddcl_init_spin(ddcl_SpinLock * l) {
#ifdef DDSYS_WIN
    l->lock = FALSE;
#else
    pthread_spin_init(&l->lock);
#endif

}

DDCLAPI int
ddcl_lock_spin(ddcl_SpinLock * l) {
#ifdef DDSYS_WIN
    while (InterlockedExchange(&l->lock, TRUE) == TRUE)
        continue;
    return 0;
#else
    return pthread_spin_lock(&l->lock);
#endif
}

DDCLAPI int
ddcl_try_lock_spin(ddcl_SpinLock * l) {
#ifdef DDSYS_WIN
    return InterlockedExchange(&l->lock, TRUE) != TRUE;
#else
    return pthread_spin_trylock(&l->lock);
#endif
}

DDCLAPI int
ddcl_unlock_spin(ddcl_SpinLock * l) {
#ifdef DDSYS_WIN
    InterlockedExchange(&l->lock, FALSE);
    return 0;
#else
    return pthread_spin_unlock(&l->lock);
#endif
}

DDCLAPI void
ddcl_destroy_spin(ddcl_SpinLock * l) {
#ifdef DDSYS_WIN
    InterlockedExchange(&l->lock, FALSE);
#else
    pthread_spin_destroy(&l->lock);
#endif
}


DDCLAPI void
ddcl_init_mutex(ddcl_Mutex * l) {
#ifdef DDSYS_WIN
    InitializeCriticalSection(&l->lock);
#else
    pthread_mutex_init(&l->lock, NULL);
#endif
}

DDCLAPI int
ddcl_lock_mutex(ddcl_Mutex * l) {
#ifdef DDSYS_WIN
    EnterCriticalSection(&l->lock);
    return 0;
#else
    return pthread_mutex_lock(&l->lock);
#endif
}

DDCLAPI int
ddcl_unlock_mutex(ddcl_Mutex * l) {
#ifdef DDSYS_WIN
    LeaveCriticalSection(&l->lock);
    return 0;
#else
    return pthread_mutex_unlock(&l->lock);
#endif
}


DDCLAPI void
ddcl_destroy_mutex(ddcl_Mutex * l) {
#ifdef DDSYS_WIN
    DeleteCriticalSection(&l->lock);
#else
    pthread_mutex_destroy(&l->lock);
#endif
}


DDCLAPI void
ddcl_init_cond (ddcl_Cond * cond) {
#ifdef DDSYS_WIN
    cond->evt = CreateSemaphore(NULL, 0, 1, NULL);
    //cond->evt = CreateEvent(NULL, FALSE, FALSE, "ddcl_Cond");
    //cond->lock = FALSE;
#else
    pthread_mutex_init(&(cond->mutex), NULL);
    pthread_cond_init(&(cond->cond), NULL);
#endif
}

DDCLAPI void
ddcl_destroy_cond(ddcl_Cond * cond) {
#ifdef DDSYS_WIN
    CloseHandle(cond->evt);
#else
    pthread_mutex_destroy(&(cond->mutex));
    pthread_cond_destroy(&(cond->cond));
#endif
}

DDCLAPI int
ddcl_wait_cond (ddcl_Cond * cond) {
#ifdef DDSYS_WIN
    /*
    while (InterlockedExchange(&cond->lock, TRUE) == TRUE) {
        ddcl_sleepms(1);
        continue;
    }
    return 0;
    */
    return WaitForSingleObject(cond->evt, INFINITE);
#else
    pthread_mutex_lock(&(cond->mutex));
    int ret = pthread_cond_wait(&(cond->cond), &(cond->mutex));
    pthread_mutex_unlock(&(cond->mutex));
    return ret;
#endif
}

DDCLAPI int
ddcl_wait_time_cond (ddcl_Cond * cond, dduint32 ms) {
#ifdef DDSYS_WIN
    /*
    long long start = ddcl_now();
    long long now;
    while (InterlockedExchange(&cond->lock, TRUE) == TRUE) {
        ddcl_sleepms(1);
        now = ddcl_now();
        if ((now - start) >= ms)
            return 1;
    }
    return 0;
    */
    return WaitForSingleObject(cond->evt, (DWORD)ms);
#else
    struct timeval now;
    gettimeofday(&now, NULL);
    struct timespec to;
    to.tv_sec = now.tv_sec + (int)(ms/1000);
    to.tv_nsec = (now.tv_usec * 1000) + (ms % 1000) * 1000000;
    pthread_mutex_lock(&(cond->mutex));
    int ret = pthread_cond_timedwait(&(cond->cond), &(cond->mutex), &to);
    pthread_mutex_unlock(&(cond->mutex));
    return ret;
#endif
}

DDCLAPI int
ddcl_signal_cond(ddcl_Cond * cond) {
#ifdef DDSYS_WIN
    /*
    InterlockedExchange(&(cond->lock), FALSE);
    return 0;
    */
    return ReleaseSemaphore(cond->evt, 1, NULL);
    //return SetEvent(cond->evt);
#else
    pthread_mutex_lock(&(cond->mutex));
    int ret = pthread_cond_signal(&(cond->cond));
    pthread_mutex_unlock(&(cond->mutex));
    return ret;
#endif
}


DDCLAPI void
ddcl_init_rw (ddcl_RWLock * l){
#ifdef DDSYS_WIN
    InitializeSRWLock(&(l->lock));
    //InitializeCriticalSection(&l->lock);
#else
    pthread_rwlock_init(&(l->lock), NULL);
#endif
}

DDCLAPI void
ddcl_destroy_rw(ddcl_RWLock * l){
#ifdef DDSYS_WIN
    //DeleteCriticalSection(&(l->lock));
#else
    pthread_rwlock_destroy(&(l->lock));
#endif
}


DDCLAPI void
ddcl_rlock_rw (ddcl_RWLock * l){
#ifdef DDSYS_WIN
    AcquireSRWLockExclusive(&(l->lock));
    //EnterCriticalSection(&l->lock);
#else
    pthread_rwlock_rdlock(&(l->lock));
#endif
}

DDCLAPI void
ddcl_wlock_rw (ddcl_RWLock * l){
#ifdef DDSYS_WIN
    AcquireSRWLockShared(&(l->lock));
    //EnterCriticalSection(&l->lock);
#else
    pthread_rwlock_wrlock(&(l->lock));
#endif
}

DDCLAPI void
ddcl_runlock_rw(ddcl_RWLock * l) {

#ifdef DDSYS_WIN
    ReleaseSRWLockExclusive(&(l->lock));
    //LeaveCriticalSection(&l->lock);
#else
    pthread_rwlock_unlock(&(l->lock));
#endif
}

DDCLAPI void
ddcl_wunlock_rw (ddcl_RWLock * l){
#ifdef DDSYS_WIN
    ReleaseSRWLockShared(&(l->lock));
    //LeaveCriticalSection(&l->lock);
#else
    pthread_rwlock_unlock(&(l->lock));
#endif
}
