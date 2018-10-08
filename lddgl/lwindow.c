#define DDGLLUA_CORE

#include "ddgl.h"
#include "lgl.h"
#include "ddglwindow.h"

static int
l_new_window (lua_State * L){
    unsigned width = luaL_checkinteger(L, 1);
    unsigned height = luaL_checkinteger(L, 2);
    ddgl_Window h = ddgl_new_window(width, height);
    lua_pushinteger(L, h);
    return 1;
}

static int
l_show_window(lua_State * L){
    ddgl_Window h = luaL_checkinteger(L, 1);
    ddgl_show_window(h);
    return 0;
}

static int
l_poll_window_event(lua_State * L){
    ddgl_poll_window_event();
    return 0;
}

static int
l_set_window_size(lua_State * L){
    ddgl_Window h = luaL_checkinteger(L, 1);
    int width = luaL_checkinteger(L, 2);
    int height = luaL_checkinteger(L, 3);
     ddgl_set_window_size(h, width, height);
    return 0;
}


static int
l_render_window(lua_State * L){
    ddgl_Window h = luaL_checkinteger(L, 1);
    ddgl_render_window(h);
    return 0;
}

static int
l_swap_buffer(lua_State * L){
    ddgl_Window h = luaL_checkinteger(L, 1);
    ddgl_swap_buffer(h);
    return 0;
}

int
open_window_module(lua_State * L){
    DDLUA_PUSHFUNC(L, "new_window", l_new_window);
    DDLUA_PUSHFUNC(L, "show_window", l_show_window);
    DDLUA_PUSHFUNC(L, "poll_window_event", l_poll_window_event);
    DDLUA_PUSHFUNC(L, "set_window_size", l_set_window_size);
    DDLUA_PUSHFUNC(L, "render_window", l_render_window);
    DDLUA_PUSHFUNC(L, "swap_buffer", l_swap_buffer);

    return 0;
}
