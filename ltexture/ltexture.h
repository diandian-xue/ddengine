#pragma once

#include "ddplatform.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#ifdef DDSYS_WIN
    #ifdef LTEXTURE_EXPORTS
        #ifdef LTEXTURELUA_CORE
            #define LTEXTURELUA    __declspec(dllexport)
        #else
            #define LTEXTURELUA    __declspec(dllimport)
        #endif
    #else
        #define LTEXTURELUA extern
    #endif
#else
    #ifdef LTEXTURE_EXPORTS
        #define LTEXTURELUA extern
    #else
        #define LTEXTURELUA
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

LTEXTURELUA int
luaopen_ltexture_core (lua_State * L);

