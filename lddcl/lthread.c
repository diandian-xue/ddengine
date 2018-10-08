#define DDCLLUA_CORE

#include "lcl.h"
#include "ddclmalloc.h"
#include "ddclthread.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct tag_Param{
    char * script;
    char * param;
}Param;

static void *
_exc_thread_fn(Param * p){
    lua_State * L = luaL_newstate();
    luaL_openlibs(L);
    int ret = luaL_loadstring(L, p->script);
    if (ret){
        printf("loadstring error %d: %s\n",ret, lua_tostring(L, -1));
        goto T_END;
    }
    lua_pushstring(L, p->param);
    ret = lua_pcall(L, 1, 0, 0);
    if (ret){
        const char * err = lua_tostring(L, -1);
        luaL_traceback(L, L, err, -1);
        const char * traceback = lua_tostring(L, -1);
        printf("thread do string err:%s\n", traceback);
    }
T_END:  
    ddcl_free(p->script);
    ddcl_free(p->param);
    ddcl_free(p);
    lua_close(L);
    ddcl_exit_thread();
    return NULL;
}

static int
l_new_thread(lua_State * L){
    size_t script_sz;
    size_t param_sz;
    const char * script = lua_tolstring(L, 1, &script_sz);
    const char * param = lua_tolstring(L, 2, &param_sz);
    int detach = lua_toboolean(L, 3);

    Param * p = ddcl_malloc(sizeof(Param));
    p->script = ddcl_malloc(script_sz + 1);
    memcpy(p->script, script, script_sz + 1);
    p->param = ddcl_malloc(param_sz + 1);
    memcpy(p->param, param, param_sz + 1);

    ddcl_Thread * t = lua_newuserdata(L, sizeof(ddcl_Thread));
    ddcl_new_thread(t, _exc_thread_fn, p, detach);
    return 1;
}

static int
l_cancel_thread(lua_State * L){
    ddcl_Thread * t = lua_touserdata(L, 1);
    lua_pushinteger(L, ddcl_cancel_thread(*t));
    return 1;
}

static int
l_join_thread(lua_State * L){
    ddcl_Thread * t = lua_touserdata(L, 1);
    lua_pushinteger(L, ddcl_join_thread(*t));
    return 1;
}

static int
l_sleepms(lua_State * L){
    dduint32 ms = (dduint32)luaL_checkinteger(L, 1);
    ddcl_sleepms(ms);
    return 0;
}

int
openlib_thread (lua_State * L){
    DDLUA_PUSHFUNC(L, "new_thread", l_new_thread);
    DDLUA_PUSHFUNC(L, "cancel_thread", l_cancel_thread);
    DDLUA_PUSHFUNC(L, "join_thread", l_join_thread);
    DDLUA_PUSHFUNC(L, "sleepms", l_sleepms);
    return 1;
}
