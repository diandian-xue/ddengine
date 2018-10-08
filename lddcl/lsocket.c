#define DDCLLUA_CORE

#include "lcl.h"
#include "lservice.h"

#include "ddclsocket.h"
#include "ddclerr.h"

#include <stdio.h>
#include <stdlib.h>

static int
l_parse_socket_rsp (lua_State * L){
    size_t sz;
    const char * data = luaL_checklstring(L, 1, &sz);
    ddcl_SocketRsp * rsp = (ddcl_SocketRsp *)data;

    lua_pushinteger(L, rsp->fd);
    lua_pushinteger(L, rsp->cmd);
    if(rsp->cmd == DDCL_SOCKET_READ){
        lua_pushlstring(L, data + sizeof(ddcl_SocketRsp), sz - sizeof(ddcl_SocketRsp));
        return 3;
    }else{
        return 2;
    }
}

static int
l_connect_socket (lua_State * L){
    FIND_CTX;
    const char * host = luaL_checkstring(L, 1);
    dduint16 port = (dduint16)luaL_checkinteger(L, 2);
    ddcl_Session session;
    int err = ddcl_new_session(ctx->svr, &session);
    if(err){
        return luaL_error(L, ddcl_err(err));
    }

    ddcl_Socket fd = ddcl_connect_socket(ctx->svr, host, port, session);
    if(fd == 0){
        return luaL_error(L, "can not connect to %s:%d", host, port);
    }
    return lservice_yield_for_session(L, ctx, session);
}

static int
l_listen_socket (lua_State * L){
    FIND_CTX;

    const char * host = luaL_checkstring(L, 1);
    dduint16 port = (dduint16)luaL_checkinteger(L, 2);
    int backlog = (int)luaL_checkinteger(L, 3);

    ddcl_Session session;
    int err = ddcl_new_session(ctx->svr, &session);
    if(err){
        return luaL_error(ctx->L, ddcl_err(err));
    }
    ddcl_Socket fd = ddcl_listen_socket(ctx->svr, host, port, backlog, session);
    if(!fd){
        lua_pushinteger(L, fd);
        return 1;
    }
    lua_pushinteger(L, fd);
    return 1;
}

static int
l_accept_socket (lua_State * L){
    FIND_CTX;
    ddcl_Socket fd = (ddcl_Socket)luaL_checkinteger(L, 1);

    ddcl_Session session;
    int err = ddcl_accept_socket(fd, ctx->svr, &session);
    if(err){
        return luaL_error(L, ddcl_err(err));
    }
    return lservice_yield_for_session(L, ctx, session);
}

static int
l_read_socket (lua_State * L){
    FIND_CTX;

    ddcl_Socket fd = (ddcl_Socket)luaL_checkinteger(L, 1);
    size_t sz = luaL_checkinteger(L, 2);

    ddcl_Session session;
    int err = ddcl_read_socket(fd, ctx->svr, sz, &session);
    if(err){
        return luaL_error(L, ddcl_err(err));
    }
    return lservice_yield_for_session(L, ctx, session);
}

static int
l_send_socket (lua_State * L){
    ddcl_Socket fd = (ddcl_Socket)luaL_checkinteger(L, 1);
    size_t sz;
    const char * data = luaL_checklstring(L, 2, &sz);
    int err = ddcl_send_socket(fd, 0, data, sz);
    if(err){
        return luaL_error(L, ddcl_err(err));
    }
    return 0;
}

static int
l_close_socket (lua_State * L){
    ddcl_Socket fd = (ddcl_Socket)luaL_checkinteger(L, 1);
    int err = ddcl_close_socket(fd, 0);
    if(err){
        return luaL_error(L, ddcl_err(err));
    }
    return 0;
}

static int
l_getall_socket_count (lua_State * L){
    lua_pushinteger(L, ddcl_getall_socket_count());
    return 1;
}

static int
l_forward_socket (lua_State * L){
    FIND_CTX;
    ddcl_Socket fd = (ddcl_Socket)luaL_checkinteger(L, 1);
    int err = ddcl_forward_socket(fd, ctx->svr);
    if(err){
        return luaL_error(L, ddcl_err(err));
    }
    return 0;
}

int
openlib_socket(lua_State * L){
    DDLUA_PUSHFUNC(L, "parse_socket_rsp", l_parse_socket_rsp);
    DDLUA_PUSHFUNC(L, "connect_socket", l_connect_socket);
    DDLUA_PUSHFUNC(L, "listen_socket", l_listen_socket);
    DDLUA_PUSHFUNC(L, "accept_socket", l_accept_socket);
    DDLUA_PUSHFUNC(L, "read_socket", l_read_socket);
    DDLUA_PUSHFUNC(L, "send_socket", l_send_socket);
    DDLUA_PUSHFUNC(L, "close_socket", l_close_socket);
    DDLUA_PUSHFUNC(L, "getall_socket_count", l_getall_socket_count);
    DDLUA_PUSHFUNC(L, "forward_socket", l_forward_socket);


    DDLUA_PUSHENUM(L, "DDCL_SOCKET_ACCEPT", DDCL_SOCKET_ACCEPT);
    DDLUA_PUSHENUM(L, "DDCL_SOCKET_READ", DDCL_SOCKET_READ);
    DDLUA_PUSHENUM(L, "DDCL_SOCKET_ERROR", DDCL_SOCKET_ERROR);

    return 0;
}
