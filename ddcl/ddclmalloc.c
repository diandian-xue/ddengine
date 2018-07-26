#define DDCL_CORE

#include "ddclmalloc.h"
#include "ddclmap.h"
#include "ddclthread.h"

#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

static ddcl_SpinLock _lock;
static ddcl_Map * _map;


DDCLAPI int
ddcl_malloc_module_init(){
    _map = ddcl_new_map(NULL, NULL);
    ddcl_init_spin(&_lock);
    return 0;
}

DDCLAPI void *
ddcl_mallocR(size_t sz, char * r, int line){
    void * p = malloc(sz);
    char * s = malloc(strlen(r) + 1 + 16);
    sprintf(s, "%s:%d", r, line);
    printf("ddcl_mallocR: %zu %p %s \n", sz, p, s);
    fflush(stdout);
    ddcl_lock_spin(&_lock);
    ddcl_set_map(_map, &p, sizeof(void*), s, 0, NULL);
    ddcl_unlock_spin(&_lock);

    return p;
}

DDCLAPI void
ddcl_freeR(void * p){
    free(p);

    ddcl_lock_spin(&_lock);
    char * s = (char*)ddcl_set_map(_map, &p, sizeof(void *), NULL, 0, NULL);
    ddcl_unlock_spin(&_lock);
    if (s){
        printf("ddcl_freeR:%p %s\n", p, s);
        fflush(stdout);
        ddcl_free(s);
    }
}

DDCLAPI void *
ddcl_realloc(void * p, size_t sz){
    void * np = realloc(p, sz);

    ddcl_lock_spin(&_lock);

    return np;
}
