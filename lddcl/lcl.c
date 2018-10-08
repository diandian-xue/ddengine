#define DDCLLUA_CORE

#include "ddcl.h"
#include "lcl.h"
#include "ddcltimer.h"

#include <stdio.h>

static int
_getconfig_dduint32(lua_State * L, const char * name, dduint32 * v, int index){
    lua_pushstring(L, name);
    lua_rawget(L, index);
    if(!lua_isnil(L, -1)){
        *v = (dduint32)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);
    return 0;
}

static int
l_init (lua_State * L){
    ddcl conf;
    ddcl_default(&conf);
    if (lua_gettop(L) > 0){
        _getconfig_dduint32(L, "worker", &conf.worker, 1);
        _getconfig_dduint32(L, "socket", &conf.socket, 1);
        _getconfig_dduint32(L, "timer_ms", &conf.timer_ms, 1);
    }
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

static int
l_packstring(lua_State * L){
    void * data = lua_touserdata(L, 1);
    if(lua_gettop(L) > 1){
        if(data){
            lua_Integer sz = luaL_checkinteger(L, 2);
            lua_pushlstring(L, (const char *)data, sz);
            return 1;
        }
    }else{
        if(data){
            lua_pushstring(L, (const char *)data);
            return 1;
        }
    }
    return 0;
}

static luaL_Reg _reg[] = {
    { "init", l_init },
    { "now", l_now },
    { "systime", l_systime },
    { "packstring", l_packstring },
    { NULL, NULL },
};


extern int
openlib_file (lua_State * L);

extern int
openlib_thread (lua_State * L);

extern int
openlib_service (lua_State * L);

extern int
openlib_socket (lua_State * L);

DDCLLUA int
luaopen_lddcl_core(lua_State * L){
    luaL_checkversion(L);
    DDLUA_NEWLIB(L, "lddcl.core", _reg);

    openlib_file(L);
    openlib_thread(L);
    openlib_service(L);
    openlib_socket(L);

    return 1;
}
