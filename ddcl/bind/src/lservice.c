#define CLLUA_CORE

#include "lcl.h"
#include "ddclservice.h"

#include <stdlib.h>
#include <stdio.h>

static void
_service_msg_cb_fn(ddcl_Msg * msg){
    lua_State * L = msg->ud;
    lua_pushstring(L, "ddcl.service.callback.fn");
    lua_rawget(L, LUA_REGISTRYINDEX);
    const char * t = lua_typename(L, lua_type(L, -1));
    if(!lua_isfunction(L, -1)){
        printf("msg cb not found function  %s\n", t);
        lua_pop(L, 1);
        return;
    }
    lua_pushinteger(L, msg->self);
    lua_pushinteger(L, msg->from);
    lua_pushinteger(L, msg->ptype);
    lua_pushinteger(L, msg->cmd);
    lua_pushinteger(L, msg->session);
    if (msg->data)
        lua_pushlstring(L, msg->data, msg->sz);
    else
        lua_pushlstring(L, "", 0);

    if(lua_pcall(L, 6, 0, 0)){
        printf("lua_pcall error: %s\n", lua_tostring(L, -1));
        luaL_error(L, "%s", lua_tostring(L, -1));
    }
}

static int
l_new_service (lua_State * L){
    size_t script_sz;
    size_t param_sz;
    const char * script = lua_tolstring(L, 1, &script_sz);
    const char * param = lua_tolstring(L, 2, &param_sz);

    lua_State * nL = luaL_newstate();
    luaL_openlibs(nL);
    if(luaL_loadstring(nL, script))
        luaL_error(L, "%s", lua_tostring(nL, -1));
    ddcl_Handle h = ddcl_new_service(_service_msg_cb_fn, nL);
    lua_pushinteger(nL, h);
    lua_pushstring(nL, param);
    if(lua_pcall(nL, 2, 0, 0))
        luaL_error(L, "%s", lua_tostring(nL, -1));
    
    lua_pushinteger(L, h);
    return 1;
}

static int
l_new_service_not_worker (lua_State * L){
    size_t script_sz;
    size_t param_sz;
    const char * script = lua_tolstring(L, 1, &script_sz);
    const char * param = lua_tolstring(L, 2, &param_sz);
    
    lua_State * nL = luaL_newstate();
    luaL_openlibs(nL);
    if(luaL_loadstring(nL, script))
        luaL_error(L, "%s", lua_tostring(nL, -1));
    ddcl_Handle h = ddcl_new_service_not_worker(_service_msg_cb_fn, nL);
    lua_pushinteger(nL, h);
    lua_pushstring(nL, param);
    if(lua_pcall(nL, 2, 0, 0))
        luaL_error(L, "%s", lua_tostring(nL, -1));
    lua_pushinteger(L, h);
    return 1;
}

static int
l_exit_service (lua_State * L){
    ddcl_Handle h = luaL_checkinteger(L, 1);
    lua_pushinteger(L, ddcl_exit_service(h));
    return 1;
}

static int
l_send(lua_State * L){
    ddcl_Handle to = luaL_checkinteger(L, 1);
    ddcl_Handle from = luaL_checkinteger(L, 2);
    int ptype = luaL_checkinteger(L, 3);
    int cmd = luaL_checkinteger(L, 4);
    ddcl_Session session = luaL_checkinteger(L, 5);
    size_t sz = 0;
    const char * data = lua_tolstring(L, 6, &sz);
    int ret = ddcl_send(to, from, ptype, cmd, session, data, sz + 1);
    lua_pushinteger(L, ret);
    return 1;
}

static int
l_call (lua_State * L){
    ddcl_Handle to = luaL_checkinteger(L, 1);
    ddcl_Handle from = luaL_checkinteger(L, 2);
    int ptype = luaL_checkinteger(L, 3);
    int cmd = luaL_checkinteger(L, 4);
    size_t sz = 0;
    const char * data = lua_tolstring(L, 5, &sz);
    ddcl_Session session;
    int ret = ddcl_call(to, from, ptype, cmd, &session, data, sz);
    lua_pushinteger(L, ret);
    lua_pushinteger(L, session);
    return 2;
}

static int
l_loop_with_not_worker (lua_State * L){
    ddcl_Handle h = luaL_checkinteger(L, 1);
    lua_pushinteger(L, ddcl_start(h));
    return 1;
}

static int
l_dispatch (lua_State * L){
    ddcl_Handle h = luaL_checkinteger(L, 1);
    lua_pushinteger(L, ddcl_dispatch(h));
    return 1;
}

static int
l_dispatch_time_wait (lua_State * L){
    ddcl_Handle h = luaL_checkinteger(L, 1);
    dduint32 ms = luaL_checkinteger(L, 2);
    lua_pushinteger(L, ddcl_dispatch_time_wait(h, ms));
    return 1;
}

static int
l_timeout (lua_State * L){
    ddcl_Handle h = luaL_checkinteger(L, 1);
    dduint32 ms = luaL_checkinteger(L, 2);
    ddcl_Session session;
    lua_pushinteger(L, ddcl_timeout(h, &session, ms));
    lua_pushinteger(L, session);
    return 2;
}

static int
l_callback (lua_State * L){
    luaL_checktype(L, 1, LUA_TFUNCTION);
    lua_pushstring(L, "ddcl.service.callback.fn");
    lua_pushvalue(L, 1);
    lua_rawset(L, LUA_REGISTRYINDEX);
    return 0;
}


static luaL_Reg _REG[] = {
    { "new_service", l_new_service },
    { "new_service_not_worker", l_new_service_not_worker },
    { "exit_service", l_exit_service },
    { "send", l_send },
    { "call", l_call },
    { "loop_with_not_worker", l_loop_with_not_worker },
    { "dispatch", l_dispatch },
    { "dispatch_time_wait", l_dispatch_time_wait },
    { "timeout", l_timeout },
    { "callback", l_callback },
    { NULL, NULL },
};


CLLUA int
luaopen_lddcl_service (lua_State * L){
    luaL_checkversion(L);
    DDLUA_NEWLIB(L, "lddcl.service", _REG);
    DDLUA_PUSHENUM(L, "PTYPE_SEND", DDCL_PTYPE_SEND);
    DDLUA_PUSHENUM(L, "PTYPE_RESP", DDCL_PTYPE_RESP);

    DDLUA_PUSHENUM(L, "CMD_TEXT", DDCL_CMD_TEXT);
    DDLUA_PUSHENUM(L, "CMD_ERROR", DDCL_CMD_ERROR);
    DDLUA_PUSHENUM(L, "CMD_TIMEOUT", DDCL_CMD_TIMEOUT);
    DDLUA_PUSHENUM(L, "CMD_SOCKET", DDCL_CMD_SOCKET);
    return 1;
}
