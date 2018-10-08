#define DDCLLUA_CORE

#include "lcl.h"
#include "lservice.h"
#include "ddclservice.h"
#include "ddclmalloc.h"
#include "ddcllog.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

static void
_print_vaule (lua_State * L, char * msg){
    lua_getglobal(L, "tostring");
    lua_pushvalue(L, -2);
    lua_call(L, 1, 1);
    const char * f = lua_tostring(L, -1);
    printf("pv: %s= %s\n", msg, f);
    lua_pop(L, 1);
}

int
lservice_yield_for_session(lua_State * L, Context * ctx, ddcl_Session session){
    lua_rawgetp(L, LUA_REGISTRYINDEX, &ctx->session_map);
    lua_pushinteger(L, session);
    lua_pushthread(L);
    lua_rawset(L, -3);
    lua_pop(L, 1);
    return lua_yield(L, 0);
}

static void
_new_session_table(lua_State * L, ddcl_Service from, ddcl_Session session){
    lua_newtable(L);
    lua_pushinteger(L, from);
    lua_rawseti(L, -2, 1);
    lua_pushinteger(L, session);
    lua_rawseti(L, -2, 2);
}

static int
_callmsg_in_coroutine(ddcl_Msg * msg, void * fn_flag){
    Context * ctx = (Context *)msg->ud;
    lua_State * L = lua_newthread(ctx->L);
    if(fn_flag){
        lua_rawgetp(L, LUA_REGISTRYINDEX, fn_flag);
        if(lua_isnil(L, -1)){
            lua_pop(L, 1);
            ddcl_log(ctx->svr,
                    "error: msg call is not in service %p", fn_flag);
            return 0;
        }
    }
    else{
        lua_rawgetp(L, LUA_REGISTRYINDEX, &ctx->session_map);
        lua_pushinteger(L, msg->session);
        lua_rawget(L, -2);
        if(lua_isnil(L, -1)){
            lua_pop(L, 2);
            ddcl_log(ctx->svr,
                    "error:not founded function in session[%d]", msg->session);
            return 0;
        }
        lua_remove(L, -2);
    }

    if(msg->from && msg->session){
        lua_rawgetp(L, LUA_REGISTRYINDEX, &ctx->co_map);
        _new_session_table(L, msg->from, msg->session);
        lua_rawsetp(L, -2, L);
        lua_pop(L, 1);
    }

    lua_pushinteger(L, msg->self);
    lua_pushinteger(L, msg->from);
    lua_pushinteger(L, msg->session);
    lua_pushinteger(L, msg->ptype);
    lua_pushinteger(L, msg->cmd);
    lua_pushlightuserdata(L, (void *)msg->data);
    lua_pushinteger(L, msg->sz);

    int ret = lua_resume(L, NULL, 7);
    switch(ret){
        case LUA_OK:
            break;
        case LUA_YIELD:
            /*
            if(msg->session && false){
                lua_rawgetp(L, LUA_REGISTRYINDEX, &ctx->session_map);
                lua_pushinteger(L, msg->session);
                lua_pushvalue(L, -2);
                lua_settable(L, -3);
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
             */
            break;
        default:{
            size_t errsz;
            const char * errstr = lua_tolstring(L, -1, &errsz);
            luaL_traceback(L, L, errstr, 0);
            const char * tb_errstr = lua_tostring(L, -1);
            ddcl_log(ctx->svr, tb_errstr);
            if (msg->from && msg->session){
                ddcl_send_b(msg->from, 0, DDCL_PTYPE_RESP,
                    DDCL_CMD_ERROR, msg->session, NULL, 0);
            }
            break;
        }
    }
    return 0;
}

static int
_excute_send_msg(ddcl_Msg * msg){
    Context * ctx = (Context *)msg->ud;
    if(msg->cmd == DDCL_CMD_LUA_START){
        return _callmsg_in_coroutine(msg, &ctx->startfn);
    }else{
        return _callmsg_in_coroutine(msg, &ctx->callback);
    }
}

static int
_excute_resp_msg(ddcl_Msg * msg){
    Context * ctx = (Context *)msg->ud;
    lua_State * L = ctx->L;
    if (!msg->session){
        ddcl_log(ctx->svr, "resp msg but session == 0");
        return 0;
    }

    lua_rawgetp(L, LUA_REGISTRYINDEX, &ctx->session_map);
    lua_pushinteger(L, msg->session);
    lua_rawget(L, -2);

    switch(lua_type(L, -1)){
        case LUA_TFUNCTION:
            return _callmsg_in_coroutine(msg, NULL);
        case LUA_TTHREAD:
            break;
        default:
            ddcl_log(ctx->svr, "unknow session[%d] for resp msg", msg->session);
            return 0;
    }

    lua_State * nL = lua_tothread(L, -1);
    if(msg->cmd == DDCL_CMD_ERROR){
        lua_rawgetp(L, LUA_REGISTRYINDEX, &ctx->session_map);
        lua_pushinteger(L, msg->session);
        lua_pushnil(L);
        lua_rawset(L, -3);
        luaL_traceback(L, nL, msg->data, 0);
        ddcl_log(ctx->svr, "call faild: %s", lua_tostring(L, -1));
        return 0;
    }
    lua_settop(nL, 0);
    lua_pushlightuserdata(nL, (void *)msg->data);
    lua_pushinteger(nL, msg->sz);
    int ret = lua_resume(nL, NULL, 2);
    switch(ret){
        case LUA_OK:
            lua_rawgetp(L, LUA_REGISTRYINDEX, &ctx->session_map);
            lua_pushinteger(L, msg->session);
            lua_pushnil(L);
            lua_rawset(L, -3);
            lua_pop(L, 1);
            break;
        case LUA_YIELD:
            break;
        default:
            ddcl_log(ctx->svr, "%s  %d", lua_tostring(nL, -1), lua_gettop(nL));
            lua_rawgetp(L, LUA_REGISTRYINDEX, &ctx->session_map);
            lua_pushinteger(L, msg->session);
            lua_pushnil(L);
            lua_rawset(L, -3);
            break;
    }
    return 0;
}

static int
l_msg_cbfn(ddcl_Msg * msg){
    Context * ctx = (Context *)msg->ud;
    lua_State * L = ctx->L;
    int top = lua_gettop(L);
    switch(msg->ptype){
        case DDCL_PTYPE_SEND:
            _excute_send_msg(msg);
            break;
        case DDCL_PTYPE_RESP:
            _excute_resp_msg(msg);
            break;
        default:
            _excute_send_msg(msg);
            break;
    }
    lua_settop(L, top);
    return 0;
}

static void
l_exit_service_cb(ddcl_Service h, void * ud){
    Context * ctx = (Context *)ud;
    lua_State * L = ctx->L;
    lua_rawgetp(L, LUA_REGISTRYINDEX, &ctx->co_map);
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        lua_rawgeti(L, -1, 1);
        ddcl_Service from = (ddcl_Service)lua_tointeger(L, -1);
        lua_pop(L, 1);

        lua_rawgeti(L, -1, 2);
        ddcl_Session session = lua_tointeger(L, -1);
        lua_pop(L, 1);
        const char * err = "service is closed";
        ddcl_send_b(from, ctx->svr, DDCL_PTYPE_RESP, 
            DDCL_CMD_ERROR, session, (void *)err, strlen(err) + 1);

        lua_pop(L, 1);
     }

    lua_close(L);
    ddcl_free(ctx);
}

static int
l_new_service (lua_State * L){
    const char * script = luaL_checkstring(L, 1);
    const char * param = luaL_checkstring(L, 2);

    FIND_CTX;

    Context * new_ctx = ddcl_malloc(sizeof(Context));
    memset(new_ctx, 0, sizeof(Context));
    new_ctx->is_worker = 1;
    lua_State * nL = luaL_newstate();
    luaL_openlibs(nL);
    new_ctx->L = nL;
    lua_pushstring(nL, LDDCL_CTX_K);
    lua_pushlightuserdata(nL, new_ctx);
    lua_rawset(nL, LUA_REGISTRYINDEX);

    lua_getglobal(L, "package");
    lua_pushstring(L, "path");
    lua_rawget(L, -2);
    const char * path = lua_tostring(L, -1);
    lua_pushstring(L, "cpath");
    lua_rawget(L, -3);
    const char * cpath = lua_tostring(L, -1);
    lua_pop(L, 3);


    lua_getglobal(nL, "package");
    lua_pushstring(nL, "path");
    lua_pushstring(nL, path);
    lua_rawset(nL, -3);

    lua_pushstring(nL, "cpath");
    lua_pushstring(nL, cpath);
    lua_rawset(nL, -3);
    lua_pop(nL, 1);

    if(luaL_loadstring(nL, script) || !lua_pushstring(nL, param) || lua_pcall(nL, 1, 0, 0)){
        ddcl_free(ctx);
        size_t errsz;
        const char * errstr = lua_tolstring(nL, -1, &errsz);
        luaL_traceback(L, nL, errstr, 0);
        const char * tb_errstr = lua_tostring(L, -1);
        lua_close(nL);
        return luaL_error(L, "load lua error:%s", tb_errstr);
    }

    ddcl_Service svr = ddcl_new_service(l_msg_cbfn, new_ctx);
    ddcl_exit_service_cb(svr, l_exit_service_cb);
    new_ctx->svr = svr;

    lua_newtable(nL);
    lua_rawsetp(nL, LUA_REGISTRYINDEX, &new_ctx->session_map);

    lua_newtable(nL);
    lua_rawsetp(nL, LUA_REGISTRYINDEX, &new_ctx->co_map);

    ddcl_send_b(svr, ctx->svr, DDCL_PTYPE_SEND, DDCL_CMD_LUA_START, 0, NULL, 0);

    lua_pushinteger(L, svr);
    return 1;
}

static int
l_start (lua_State * L){
    luaL_checktype(L, 1, LUA_TFUNCTION);

    if(lua_gettop(L) >= 2){
        ddcl_Service svr = (ddcl_Service)luaL_checkinteger(L, 2);
        return 0;
    }

    lua_pushstring(L, LDDCL_CTX_K);
    lua_rawget(L, LUA_REGISTRYINDEX);
    if(lua_isnil(L, -1)){
        return luaL_error(L, "lddcl.start mast run in worker service");
    }
    Context * ctx = lua_touserdata(L, -1);
    lua_pushvalue(L, 1);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &ctx->startfn);
    return 0;
}

static int
l_exit (lua_State * L){
    FIND_CTX;
    ddcl_Service svr = ctx->svr;
    if(lua_gettop(L) > 0){
        svr = (ddcl_Service)luaL_checkinteger(L, 1);
    }
    ddcl_exit_service(svr);
    if(ctx->svr == svr){
        lua_yield(L, 0);
    }
    return 0;
}

static int
l_callback (lua_State * L){
    luaL_checktype(L, 1, LUA_TFUNCTION);

    FIND_CTX;
    lua_rawgetp(L, LUA_REGISTRYINDEX, &ctx->startfn);
    if(lua_type(L, -1) != LUA_TFUNCTION){
        lua_pop(L, 1);
        return luaL_error(L, "lddcl.callback mast run in service coroutine");
    }

    lua_pushvalue(L, 1);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &ctx->callback);

    lua_pop(L, 1);
    return 0;
}

static int
l_start_non_worker(lua_State * L){
    lua_pushstring(L, LDDCL_CTX_K);
    lua_rawget(L, LUA_REGISTRYINDEX);
    if(!lua_isnil(L, -1)){
        return luaL_error(L, "lddcl.start_non_worker is repeat call");
    }
    lua_pop(L, 1);

    luaL_checktype(L, 1, LUA_TFUNCTION);

    Context * ctx = ddcl_malloc(sizeof(Context));
    ctx->is_worker = 0;
    memset(ctx, 0, sizeof(Context));
    ddcl_Service svr = ddcl_new_service_not_worker(l_msg_cbfn, ctx);
    ddcl_exit_service_cb(svr, l_exit_service_cb);
    ctx->L = L;
    ctx->svr = svr;

    lua_pushstring(L, LDDCL_CTX_K);
    lua_pushlightuserdata(L, ctx);
    lua_rawset(L, LUA_REGISTRYINDEX);

    lua_pushvalue(L, 1);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &ctx->startfn);

    lua_newtable(L);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &ctx->session_map);

    lua_newtable(L);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &ctx->co_map);

    ddcl_send_b(svr, 0, DDCL_PTYPE_SEND, DDCL_CMD_LUA_START, 0, NULL, 0);
    ddcl_start(svr);

    return 0;
}

static int
l_send (lua_State * L){
    ddcl_Service to = (ddcl_Service)luaL_checkinteger(L, 1);
    size_t sz;
    const char * str = luaL_checklstring(L, 2, &sz);

    lua_pushstring(L, LDDCL_CTX_K);
    lua_rawget(L, LUA_REGISTRYINDEX);
    if(lua_isnil(L, -1)){
        return luaL_error(L, "lddcl.send mast run service coroutine");
    }

    Context * ctx = lua_touserdata(L, -1);
    ddcl_send_b(to, ctx->svr,
            DDCL_PTYPE_SEND, DDCL_CMD_TEXT, 0, (void *)str, sz);

    lua_pop(L, 1);
    return 0;
}

static int
l_call (lua_State * L){
    ddcl_Service to = (ddcl_Service)luaL_checkinteger(L, 1);
    size_t sz;
    const char * str = luaL_checklstring(L, 2, &sz);

    FIND_CTX;

    ddcl_Session session;
    int ret = ddcl_call_b(to, ctx->svr, DDCL_PTYPE_SEND,
                DDCL_CMD_TEXT, &session, (void *)str, sz);
    if(ret){
        return luaL_error(L, ddcl_err(ret));
    }
    return lservice_yield_for_session(L, ctx, session);
}

static int
l_resp (lua_State * L){
    size_t sz;
    const char * str = luaL_checklstring(L, 1, &sz);

    FIND_CTX;

    lua_rawgetp(L, LUA_REGISTRYINDEX, &ctx->co_map);
    lua_rawgetp(L, -1, L);
    if(lua_isnil(L, -1)){
        lua_pop(L, 2);
        ddcl_log(ctx->svr, "resp can not founded co_map");
        return 0;
    }
    lua_rawgeti(L, -1, 1);
    ddcl_Service from = (ddcl_Service)lua_tointeger(L, -1);
    lua_rawgeti(L, -2, 2);
    ddcl_Session session = lua_tointeger(L, -1);
    lua_pop(L, 3);

    lua_pushnil(L);
    lua_rawsetp(L, -2, L);
    lua_pop(L, 1);

    int err = ddcl_send_b(from, ctx->svr,
                DDCL_PTYPE_RESP, DDCL_CMD_TEXT, session, (void *)str, sz);
    if(err){
        lua_pop(L, 1);
        return luaL_error(L, "resp error:%s \n%s", ddcl_err(err));
    }
    lua_pop(L, 1);
    return 0;
}

static int
l_timeout(lua_State * L){
    dduint32 ms = (dduint32)luaL_checkinteger(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    FIND_CTX;

    ddcl_Session session;
    ddcl_timeout(ctx->svr, &session, ms);

    lua_rawgetp(L, LUA_REGISTRYINDEX, &ctx->session_map);
    lua_pushinteger(L, session);
    lua_pushvalue(L, 2);
    lua_rawset(L, -3);
    lua_pop(L, 1);
    return 0;
}


static int
l_log (lua_State * L){
    FIND_CTX;
    int top = lua_gettop(L);
    if(!top){
        return 0;
    }
    lua_newtable(L);
    for (int i = 1; i <= top; i ++){
        lua_getglobal(L, "tostring");
        lua_pushvalue(L, i);
        lua_call(L, 1, 1);
        lua_rawseti(L, -2, i);
    }
    lua_getglobal(L, "table");
    lua_pushstring(L, "concat");
    lua_rawget(L, -2);
    lua_pushvalue(L, -3);
    lua_pushstring(L, "  ");
    lua_call(L, 2, 1);
    const char * str = lua_tostring(L, -1);
    ddcl_log(ctx->svr, str);
    lua_pop(L, 3);
    return 0;
}

static int
l_fork (lua_State * L){
    FIND_CTX;
    luaL_checktype(L, 1, LUA_TFUNCTION);

    ddcl_Session session;
    ddcl_call(ctx->svr, ctx->svr,
            DDCL_PTYPE_RESP, DDCL_CMD_TEXT, &session, NULL, 0);

    lua_rawgetp(L, LUA_REGISTRYINDEX, &ctx->session_map);
    lua_pushinteger(L, session);
    lua_pushvalue(L, 1);
    lua_rawset(L, -3);
    lua_pop(L, 1);
    return 0;
}

static int
l_co_sleep (lua_State * L){
    FIND_CTX;

    dduint32 ms = (dduint32)luaL_checkinteger(L, 1);

    ddcl_Session session;
    ddcl_timeout(ctx->svr, &session, ms);

    lua_rawgetp(L, LUA_REGISTRYINDEX, &ctx->session_map);
    lua_pushinteger(L, session);
    lua_pushthread(L);
    lua_rawset(L, -3);
    lua_pop(L, 2);
    lua_yield(L, 0);
    return 0;
}

static int
l_co_running (lua_State * L){
    lua_pushthread(L);
    return 1;
}

static int
l_co_resume (lua_State * L){
    luaL_checktype(L, 1, LUA_TTHREAD);
    lua_State * coL = lua_tothread(L, 1);

    int top = lua_gettop(L) - 1;
    lua_settop(coL, 0);
    for (int i = 2; i <= (top + 1); i ++){
        lua_pushvalue(L, i);
    }
    lua_xmove(L, coL, top);
    int ret = lua_resume(coL, NULL, top);
    switch(ret){
        case LUA_OK:
            break;
        case LUA_YIELD:
            break;
        default:
            return luaL_error(L, lua_tostring(coL, -1));
            break;
    }
    return 0;
}

static int
l_co_yield (lua_State * L){
    lua_yield(L, 0);
    return 0;
}

int
openlib_service (lua_State * L){
    DDLUA_PUSHENUM(L, "DDCL_PTYPE_SEND", DDCL_PTYPE_SEND);
    DDLUA_PUSHENUM(L, "DDCL_PTYPE_RESP", DDCL_PTYPE_RESP);
    DDLUA_PUSHENUM(L, "DDCL_PTYPE_END", DDCL_PTYPE_END);

    DDLUA_PUSHENUM(L, "DDCL_CMD_TEXT",          DDCL_CMD_TEXT);
    DDLUA_PUSHENUM(L, "DDCL_CMD_TIMEOUT",       DDCL_CMD_TIMEOUT);
    DDLUA_PUSHENUM(L, "DDCL_CMD_ERROR",         DDCL_CMD_ERROR);
    DDLUA_PUSHENUM(L, "DDCL_CMD_SOCKET",        DDCL_CMD_SOCKET);
    DDLUA_PUSHENUM(L, "DDCL_CMD_LOG",           DDCL_CMD_LOG);
    DDLUA_PUSHENUM(L, "DDCL_CMD_LUA_START",     DDCL_CMD_LUA_START);
    DDLUA_PUSHENUM(L, "DDCL_CMD_END",           DDCL_CMD_END);



    DDLUA_PUSHFUNC(L, "new_service", l_new_service);
    DDLUA_PUSHFUNC(L, "start", l_start);
    DDLUA_PUSHFUNC(L, "exit", l_exit);
    DDLUA_PUSHFUNC(L, "callback", l_callback);
    DDLUA_PUSHFUNC(L, "start_non_worker", l_start_non_worker);
    DDLUA_PUSHFUNC(L, "send", l_send);
    DDLUA_PUSHFUNC(L, "call", l_call);
    DDLUA_PUSHFUNC(L, "resp", l_resp);
    DDLUA_PUSHFUNC(L, "timeout", l_timeout);
    DDLUA_PUSHFUNC(L, "log", l_log);
    DDLUA_PUSHFUNC(L, "fork", l_fork);
    DDLUA_PUSHFUNC(L, "co_sleep", l_co_sleep);
    DDLUA_PUSHFUNC(L, "co_running", l_co_running);
    DDLUA_PUSHFUNC(L, "co_resume", l_co_resume);
    DDLUA_PUSHFUNC(L, "co_yield", l_co_yield);

    return 0;
}
