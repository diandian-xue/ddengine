#define DDCL_CORE

#include "ddclstorage.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

typedef struct tag_Block{
    char used;
    ddcl_Handle h;
}Block;

typedef struct tag_Slot {
    char * buf;
    dduint32 size;
    struct tag_Slot * next;
}Slot;

struct tag_ddcl_Storage{
    ddcl_Handle h_index;
    dduint32 size;
    dduint32 ele_sz;
    Slot * slot;
    Slot * slot_end;
};

#define SLOT_OFFSET (1 + sizeof(ddcl_Handle))

static Slot *
_new_slot(dduint32 ele_sz, dduint32 size) {
    Slot * s = malloc(sizeof(Slot));
    s->buf = malloc((ele_sz + SLOT_OFFSET) * size);
    memset(s->buf, 0, (ele_sz + SLOT_OFFSET) * size);
    s->size = size;
    s->next = NULL;
    return s;
}

static void
_free_slot(Slot * s) {
    Slot * tmp;
    while (s) {
        tmp = s;
        free(s->buf);
        free(tmp);
        s = s->next;
    }
}

static char *
_register_in_slot(ddcl_Storage * hs, ddcl_Handle h) {
    char * buf;
    ddcl_Handle hash;
    Slot * s = hs->slot;
    while (s) {
        hash = h % s->size;
        buf = &(s->buf[hash * (SLOT_OFFSET + hs->ele_sz)]);
        if (!buf[0]) {
            buf[0] = 1;
            *(ddcl_Handle *)(&buf[1]) = h;
            return &(buf[SLOT_OFFSET]);
        }
        s = s->next;
    }
    return NULL;
}

static char *
_find_in_slot(ddcl_Storage * hs, ddcl_Handle h) {
    char * buf;
    ddcl_Handle hash;
    Slot * s = hs->slot;
    while (s) {
        hash = h % s->size;
        buf = &(s->buf[hash * (SLOT_OFFSET + hs->ele_sz)]);
        if (buf[0] && (*(ddcl_Handle *)(&buf[1]) == h)) {
            return buf;
        }
        s = s->next;
    }
    return NULL;
}

DDCLAPI ddcl_Storage *
ddcl_new_storage (unsigned ele_sz, unsigned initSize){
    if (initSize < 1)
        initSize = 1;
    ddcl_Storage * hs = malloc(sizeof(ddcl_Storage));
    memset(hs, 0, sizeof(ddcl_Storage));
    hs->h_index = 0;
    hs->size = initSize;
    hs->ele_sz = ele_sz;
    hs->slot = _new_slot(ele_sz, initSize);
    hs->slot_end = hs->slot;
    return hs;
}

DDCLAPI void
ddcl_free_storage (ddcl_Storage * hs){
    _free_slot(hs->slot);
    free(hs);
}

DDCLAPI ddcl_Handle
ddcl_register_in_storage (ddcl_Storage * hs, void ** p){
    ddcl_Handle handle;
    char * buf;
    dduint32 h_index = hs->h_index;
    for(;;){
        for(dduint32 i = 0; i < hs->size; i ++){
            handle = ++h_index;
            if (handle == 0)
                handle = ++h_index;
            buf = _register_in_slot(hs, handle);
            if(buf){
                *p = buf;
                hs->h_index = h_index;
                return handle;
            }
        }
        ddcl_expand_storage(hs, hs->slot_end->size * 2);
    }
}

DDCLAPI void *
ddcl_find_in_storage (ddcl_Storage * hs, ddcl_Handle h){
    char * buf = _find_in_slot(hs, h);
    if (buf) {
        return &(buf[SLOT_OFFSET]);
    }
    return NULL;
}

DDCLAPI int
ddcl_del_in_storage (ddcl_Storage * hs, ddcl_Handle h){
    char * buf = _find_in_slot(hs, h);
    if (buf) {
        memset(buf, 0, SLOT_OFFSET + hs->ele_sz);
        return 0;
    }
    return 1;
}

DDCLAPI void
ddcl_expand_storage (ddcl_Storage * hs, dduint32 size){
    Slot * s = _new_slot(hs->ele_sz, size);
    hs->slot_end->next = s;
    hs->slot_end = s;
    hs->size += size;
}
