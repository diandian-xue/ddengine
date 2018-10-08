#define DDGLLUA_CORE

#include "ddgl.h"
#include "lgl.h"
#include "ddglwindow.h"

#include <stdio.h>


static int
l_init(lua_State * L){
    ddgl conf;
    ddgl_default(&conf);
    ddgl_init(&conf);

    return 0;
}

static int
l_init_modules(lua_State * L){
    ddgl_init_modules();
    return 0;
}

static luaL_Reg _reg[] = {
    { "init", l_init },
    { "init_modules", l_init_modules },
    { NULL, NULL },
};


extern int
open_math_module(lua_State * L);

extern int
open_window_module(lua_State * L);

extern int
open_opengl_module(lua_State * L);

extern int
open_render_module(lua_State * L);

DDGLLUA int
luaopen_lddgl_core(lua_State * L){
    luaL_checkversion(L);
    DDLUA_NEWLIB(L, "lddgl.core", _reg);

    open_math_module(L);
    open_window_module(L);
    open_opengl_module(L);
    open_render_module(L);
    return 1;
}
