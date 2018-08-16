#define DDCL_CORE

#include "ddclmap.h"
#include "ddclmalloc.h"

#include <stdlib.h>
#include <string.h>
#include <memory.h>

typedef struct tag_Entry{
    char * k;
    size_t ksz;
    void * data;
    size_t datasz;
    dduint32 hash;
    struct tag_Entry * next;
    struct tag_Entry * iterLast;
    struct tag_Entry * iterNext;
}Entry;

struct tag_ddcl_Map{
    Entry ** slot;
    dduint32 slot_sz;

    dduint32 sz;
    dduint32 load;
    dduint32 threshold;

    ddcl_MapKeyCmpFn com_fn;
    ddcl_MapKeyHashFn hash_fn;

    Entry * iter;
    Entry * iter_end;
    Entry * iter_cur;
};


static dduint32
_hashfunc_AP(const char * dat, size_t size){
     dduint32 hash = 0xAAAAAAAA;
    for(size_t i = 0; i < size; i++){
        if ((i & 1) == 0)
            hash ^= ((hash << 7) ^ dat[i] * (hash >> 3));
        else
            hash ^= (~((hash << 11) + dat[i] ^ (hash >> 5)));
    }
    return hash;
}

static dduint32
_hashfunc_RS(const char * dat, size_t size){
    int b = 378551; int a = 63689;
    dduint32 hash = 0;
    for(size_t i = 0; i < size; i++){
        hash = hash * a + dat[i];
        a = a * b;
    }
    return hash;
}

static dduint32
_hashfunc_JS(const char * dat, size_t size){
     dduint32 hash = 1315423911;
    for(size_t i = 0; i < size; i++)
        hash ^= ((hash << 5) + dat[i] + (hash >> 2));
    return hash;
}

static dduint32
_hashfunc_PJW(const char * dat, size_t size){
    dduint32 BitsInUnsignedInt = (dduint32)(4 * 8);
    dduint32 ThreeQuarters     = (dduint32)((BitsInUnsignedInt  * 3) / 4);
    dduint32 OneEighth         = (dduint32)(BitsInUnsignedInt / 8);
    dduint32 HighBits          = (dduint32)(0xFFFFFFFF) << (BitsInUnsignedInt - OneEighth);
     dduint32 hash = 0;  dduint32 test = 0;
    for(size_t i = 0; i < size; i++){
        hash = (hash << OneEighth) + dat[i];
        if((test = hash & HighBits)  != 0)
            hash = (( hash ^ (test >> ThreeQuarters)) & (~HighBits));
    }
    return hash;
}

static dduint32
_hashfunc_ELF(const char * dat, size_t size){
     dduint32 hash = 0;  dduint32 x = 0;
    for(size_t i = 0; i < size; i++){
        hash = (hash << 4) + dat[i];
        if((x = hash & 0xF0000000L) != 0)
            hash ^= (x >> 24);
        hash &= ~x;
    }
    return hash;
}

static dduint32
_hashfunc_BKDR(const char * dat, size_t size){
     dduint32 hash = 0;  dduint32 seed = 131; // 31 131 1313 13131 131313 etc..
    for(dduint32 i = 0; i < size; i++)
        hash = (hash * seed) + dat[i];
    return hash;
}

static dduint32
_hashfunc_SDBM(const char * dat, size_t size){
     dduint32 hash = 0;
    for(size_t i = 0; i < size; i++)
        hash = dat[i] + (hash << 6) + (hash << 16) - hash;
    return hash;
}

static dduint32
_hashfunc_DJB(const char * dat, size_t size){
     dduint32 hash = 5381;
    for(size_t i = 0; i < size; i++)
        hash = ((hash << 5) + hash) + dat[i];
    return hash;
}

static dduint32
_hashfunc_DEK(const char * dat, size_t size){
     dduint32 hash = (dduint32)size;
    for(size_t i = 0; i < size; i++)
        hash = ((hash << 5) ^ (hash >> 27)) ^ dat[i];
    return hash;
}

static ddcl_MapKeyHashFn g_hash_ways[] = {
    _hashfunc_AP,       // 0
    _hashfunc_RS,       // 1
    _hashfunc_JS,       // 2
    _hashfunc_PJW,      // 3
    _hashfunc_ELF,      // 4
    _hashfunc_BKDR,     // 5
    _hashfunc_SDBM,     // 6
    _hashfunc_DJB,      // 7
    _hashfunc_DEK,      // 8
};

static int
_default_cmp_fn(void * k1, size_t sz1, void * k2, size_t sz2){
    if(sz1 != sz2)
        return 0;
    return strncmp(k1, k2, sz1) == 0;
}

DDCLAPI dduint32
ddcl_hash(const char * k, size_t sz, int type){
    return g_hash_ways[type](k, sz);
}

DDCLAPI dduint32
ddcl_map_hash(const char * k, size_t sz){
    return g_hash_ways[DDHASH_BKDR](k, sz);
}

DDCLAPI ddcl_Map *
ddcl_new_map(ddcl_MapKeyCmpFn com_fn, ddcl_MapKeyHashFn hash_fn){
    ddcl_Map * map = malloc(sizeof(ddcl_Map));
    memset(map, 0, sizeof(ddcl_Map));

    dduint32 mem_sz = sizeof(Entry *) * DDCLMAP_CAPACITY;
    map->slot = malloc(mem_sz);
    memset(map->slot, 0, mem_sz);

    map->slot_sz = DDCLMAP_CAPACITY;
    map->load = DDCLMAP_LOAD;
    map->threshold = map->slot_sz * map->load;
    map->com_fn = com_fn ? com_fn : _default_cmp_fn;
    map->hash_fn = hash_fn ? hash_fn : ddcl_map_hash;
    return map;
}

DDCLAPI void
ddcl_free_map(ddcl_Map * map){
    ddcl_begin_map(map);
    void * k;
    size_t ksz;
    void * data;
    size_t datasz = 0;
    while(ddcl_next_map(map, &k, &ksz, &data, &datasz)){
        if(datasz)
            free(data);
    }
    free(map->slot);
    free(map);
}

DDCLAPI void
ddcl_expand_map(ddcl_Map * map, dduint32 sz){
    if(sz <= map->slot_sz)
        return;

    dduint32 mem_sz = sizeof(Entry *) * sz;
    Entry ** nslot = malloc(mem_sz);
    memset(nslot, 0, mem_sz);

    Entry * begin = map->iter;
    dduint32 index;
    while(begin){
        index = begin->hash % sz;
        begin->next = nslot[index];
        nslot[index] = begin;
        begin = begin->iterNext;
    }
    free(map->slot);
    map->slot = nslot;
    map->slot_sz = sz;
    map->threshold = sz * map->load;
}

DDCLAPI void *
ddcl_set_map (ddcl_Map * map, void * k, size_t ksz, 
    void * data, size_t datasz, size_t * oldsz){
    dduint32 hash = map->hash_fn((const char *)k, ksz);
    dduint32 index = hash % map->slot_sz;

    Entry * entry = map->slot[index];
    Entry * pre = NULL;
    while(entry){
        if(entry->hash != hash || !map->com_fn(entry->k, entry->ksz, k, ksz)){
            pre = entry;
            entry = entry->next;
            continue;
        }
        void * tmp = entry->data;
        if(data){
            if (datasz){
                entry->data = ddcl_malloc(datasz);
                memcpy(entry->data, data, datasz);
            }else{
                entry->data = data;
            }
            if(oldsz) *oldsz = entry->datasz;
            entry->datasz = datasz;
            return tmp;
        }
        if(pre)
            pre->next = entry->next;
        if (entry == map->slot[index])
            map->slot[index] = entry->next;
        if (map->iter == entry)
            map->iter = entry->next;
        if (map->iter_end == entry)
            map->iter_end = pre;
        if (map->iter_cur == entry)
            map->iter_cur = entry->next;
        if(entry->iterLast)
            entry->iterLast->iterNext = entry->iterNext;
        if (entry->iterNext)
            entry->iterNext->iterLast = entry->iterLast;
        free(entry->k);
        free(entry);
        map->sz --;
        return tmp;
    }

    if(!data)
        return NULL;

    entry = malloc(sizeof(Entry));
    memset(entry, 0, sizeof(Entry));
    entry->k = malloc(ksz);
    memcpy(entry->k, k, ksz);
    entry->ksz = ksz;
    if (datasz){
        entry->data = ddcl_malloc(datasz);
        memcpy(entry->data, data, datasz);
    }else{
        entry->data = data;
    }
    entry->datasz = datasz;
    entry->hash = hash;

    entry->next = map->slot[index];
    map->slot[index] = entry;
    map->sz ++;

    if(map->iter_end){
        map->iter_end->iterNext = entry;
        entry->iterLast = map->iter_end;
        map->iter_end = entry;
    }else{
        map->iter = entry;
        map->iter_end = entry;
    }
    if(map->sz >= map->threshold)
        ddcl_expand_map(map, map->slot_sz * 2);
    return NULL;
}

DDCLAPI void *
ddcl_get_map(ddcl_Map * map, void * k, size_t ksz, size_t * datasz){
    dduint32 hash = map->hash_fn(k, ksz);
    dduint32 index = hash % map->slot_sz;
    Entry * entry = map->slot[index];
    while(entry){
        if(entry->hash == hash && map->com_fn(entry->k, entry->ksz, k, ksz)){
            if (datasz) *datasz = entry->datasz;
            return entry->data;
        }
        entry = entry->next;
    }
    return NULL;
}

DDCLAPI dduint32
ddcl_get_map_size(ddcl_Map * map){
    return map->sz;
}

DDCLAPI void
ddcl_begin_map(ddcl_Map * map){
    map->iter_cur = map->iter;
}

DDCLAPI int
ddcl_next_map(ddcl_Map * map, 
    void ** key, size_t * ksz, void ** data, size_t * datasz){
    if(map->iter_cur){
        *key = map->iter_cur->k;
        *ksz = map->iter_cur->ksz;
        *data = map->iter_cur->data;
        if (datasz) *datasz = map->iter_cur->datasz;
        map->iter_cur = map->iter_cur->iterNext;
        return 1;
    }else{
        *key = NULL;
        *ksz = 0;
        *data = NULL;
        return 0;
    }
}
