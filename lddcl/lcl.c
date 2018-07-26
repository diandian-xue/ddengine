#define CLLUA_CORE

#include "ddcl.h"
#include "lcl.h"
#include "ddclservice.h"
#include "ddcltimer.h"

static int
l_init (lua_State * L){
    ddcl conf;
    ddcl_default(&conf);
    conf.worker = 2;
    int ret = ddcl_init(&conf);
    if(ret){
        luaL_error(L, ddcl_err(ret));
    }
    return 0;
}

static int
l_now (lua_State * L){
    lua_pushinteger(L, ddcl_now());
    return 1;
}

static int
l_systime (lua_State * L){
    lua_pushinteger(L, ddcl_systime());
    return 1;
}


extern int
openlib_file (lua_State * L);

extern int
openlib_thread (lua_State * L);

extern int
openlib_service (lua_State * L);

extern int
openlib_socket (lua_State * L);

static luaL_Reg _reg[] = {
    { "init", l_init },
    { "now", l_now },
    { "systime", l_systime },
    { NULL, NULL },
};


CLLUA int
luaopen_lddcl_core(lua_State * L){
    luaL_checkversion(L);
    DDLUA_NEWLIB(L, "lddcl.core", _reg);

    openlib_file(L);
    openlib_thread(L);
    openlib_service(L);
    openlib_socket(L);

    return 1;
}
