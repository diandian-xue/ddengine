#pragma once

#include "ddclservice.h"

#define LDDCL_CTX_K "lddcl_service_context"

typedef struct tag_Context{
    lua_State * L;
    ddcl_Service svr;

    int unknow;
    int startfn;
    int callback;
    int session_map;
    int co_map;
}Context;

#define FIND_CTX \
Context * ctx = NULL; \
{ \
    lua_pushstring(L, LDDCL_CTX_K); \
    lua_rawget(L, LUA_REGISTRYINDEX); \
    if(lua_isnil(L, -1)){ \
        return luaL_error(L, "not founded context"); \
    }\
    ctx = lua_touserdata(L, -1); \
    lua_pop(L, 1); \
}


int
lservice_yield_for_session(
        lua_State * L, Context * ctx, ddcl_Session session);
