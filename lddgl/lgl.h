#pragma once

#include "ddplatform.h"
#include "ddclerr.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#ifdef DDSYS_WIN
    #ifdef DDGLLUA_EXPORTS
        #ifdef DDGLLUA_CORE
            #define DDGLLUA    __declspec(dllexport)
        #else
            #define DDGLLUA    __declspec(dllimport)
        #endif
    #else
        #define DDGLLUA extern
    #endif
#else
    #ifdef DDGLLUA_EXPORTS
        #define DDGLLUA extern
    #else
        #define DDGLLUA
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

DDGLLUA int
luaopen_lddgl_core (lua_State * L);

