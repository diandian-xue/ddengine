
#include "ddclmap.h"
#include "ddclmalloc.h"

#include <stdio.h>
#include <string.h>

#define STRLEN(s)  (strlen(s) + 1)

void
map_set(ddcl_Map * map, const char * k, dduint32 * v){
    void * oldv = ddcl_set_map(map, k, STRLEN(k), v, sizeof(dduint32), NULL);
    if(oldv) ddcl_free(oldv);
}

void t_map(){
    printf("ddclmap test start ----------\n\n");

    dduint32 v1 = 1;
    dduint32 v2 = 2;
    dduint32 v3 = 3;
    dduint32 v4 = 4;
    dduint32 v5 = 5;


    ddcl_Map * map = ddcl_new_map(NULL, NULL);
    map_set(map, "diandian", &v1);
    map_set(map, "DIANDIAN", &v2);
    map_set(map, "test", &v3);
    map_set(map, "TEST", &v4);
    map_set(map, "end", &v5);

    map_set(map, "test", NULL);

    dduint32 * diandian = (dduint32 *)ddcl_get_map(map, "diandian", STRLEN("diandian"), NULL);
    dduint32 * DIANDIAN = (dduint32 *)ddcl_get_map(map, "DIANDIAN", STRLEN("DIANDIAN"), NULL);
    dduint32 * test = (dduint32 *)ddcl_get_map(map, "test", STRLEN("test"), NULL);
    dduint32 * TEST = (dduint32 *)ddcl_get_map(map, "TEST", STRLEN("TEST"), NULL);
    dduint32 * end = (dduint32 *)ddcl_get_map(map, "end", STRLEN("end"), NULL);


    printf("map find diandian  %d \n", *diandian);
    printf("map find DIANDIAN  %d \n", *DIANDIAN);
    printf("map find test  %d \n", test ? *test : 0);
    printf("map find TEST  %d \n", *TEST);
    printf("map find end  %d \n", *end);

    void * k;
    size_t sz;
    void * data;
    ddcl_begin_map(map);
    while (ddcl_next_map(map, &k, &sz, &data, NULL)) {
        printf("ddcl_next_map: %s  %zu  %d \n", k, sz, *(dduint32 *)data);
    }

    ddcl_free_map(map);

    printf("\nddclmap test ended ----------\n\n");
}
