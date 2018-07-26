#define CLLUA_CORE

#include "lcl.h"
#include "ddcltimer.h"

#include <stdlib.h>
#include <stdio.h>

static int
l_now(lua_State * L){
	lua_pushinteger(L, ddcl_now());
	return 1;
}

static int
l_systime(lua_State * L){
	lua_pushinteger(L, ddcl_systime());
	return 1;
}

static luaL_Reg _REG[] = {
    { "now", l_now },
    { "systime", l_systime },
    { NULL, NULL },
};

CLLUA int
luaopen_lddcl_timer (lua_State * L){
	luaL_checkversion(L);
    DDLUA_NEWLIB(L, "lddcl.timer", _REG);
    return 1;
}