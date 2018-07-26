﻿#pragma once

#include "ddclconfig.h"


enum ddcl_HashType{
    DDHASH_AP = 0,
    DDHASH_RS,
    DDHASH_JS,
    DDHASH_PJW,
    DDHASH_ELF,
    DDHASH_BKDR,
    DDHASH_SDBM,
    DDHASH_DJB,
    DDHASH_DEK,
};
#define DDCLMAP_CAPACITY   2
#define DDCLMAP_LOAD    2

typedef struct tag_ddcl_Map ddcl_Map;
typedef int (* ddcl_MapKeyCmpFn)
    (void * k1, dduint32 sz1, void * k2, dduint32 sz2);
typedef dduint32 (* ddcl_MapKeyHashFn)(const char * k, dduint32 sz);


DDCLAPI dduint32
ddcl_hash(const char * k, dduint32 sz, int type);

DDCLAPI dduint32
ddcl_map_hash(const char * k, dduint32 sz);

DDCLAPI ddcl_Map *
ddcl_new_map(ddcl_MapKeyCmpFn cmp_fn, ddcl_MapKeyHashFn hash_fn);

DDCLAPI void
ddcl_free_map(ddcl_Map * map);

DDCLAPI void
ddcl_expand_map(ddcl_Map * map, dduint32 sz);

DDCLAPI void *
ddcl_set_map (ddcl_Map * map, 
    void * k, size_t ksz, void * data, size_t datasz, size_t * oldsz);

DDCLAPI void *
ddcl_get_map(ddcl_Map * map, void * k, size_t ksz, size_t * datasz);

DDCLAPI dduint32
ddcl_get_map_size(ddcl_Map * map);

DDCLAPI void
ddcl_begin_map(ddcl_Map * map);

DDCLAPI int
ddcl_next_map(ddcl_Map * map, 
    void ** key, size_t * ksz, void ** data, size_t * datasz);