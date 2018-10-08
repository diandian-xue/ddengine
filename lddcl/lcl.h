#pragma once

#include "ddplatform.h"
#include "ddclerr.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#ifdef DDSYS_WIN
    #ifdef DDCLLUA_EXPORTS
        #ifdef DDCLLUA_CORE
            #define DDCLLUA    __declspec(dllexport)
        #else
            #define DDCLLUA    __declspec(dllimport)
        #endif
    #else
        #define DDCLLUA extern
    #endif
#else
    #ifdef DDCLLUA_EXPORTS
        #define DDCLLUA extern
    #else
        #define DDCLLUA
    #endif
#endif

#if LUA_VERSION_NUM == 501

#define lua_rawlen lua_objlen
#define luaL_buffinit(L , _ )
#define luaL_prepbuffsize( b , cap ) malloc(cap)
#define _Free(p) free(p)
#undef luaL_addsize
#define luaL_addsize(b , len) lua_pushlstring(L, temp , len) ; free(temp)
#define luaL_pushresult(b)
#define luaL_checkversion(L)

#define DDLUA_NEWLIB(L, NAME, REG)  luaL_register(L, NAME, REG)
#else
#define DDLUA_NEWLIB(L, NAME, REG)  luaL_newlib(L, REG)
#endif

#define DDLUA_PUSHFUNC(L, NAME, F)  lua_pushstring(L, NAME); lua_pushcfunction(L, F); lua_settable(L, -3)
#define DDLUA_PUSHENUM(L, NAME, M)  lua_pushstring(L, NAME); lua_pushinteger(L, M); lua_settable(L, -3)

DDCLLUA int
luaopen_lddcl_core (lua_State * L);

