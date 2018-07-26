#define CLLUA_CORE

#include "ddcl.h"
#include "lcl.h"

static int
l_init(lua_State * L){
    ddcl conf;
    ddcl_default(&conf);
    lua_pushinteger(L, ddcl_init(&conf));
    return 1;
}


static luaL_Reg _reg[] = {
    { "init", l_init },
    { NULL, NULL },
};

CLLUA int
luaopen_lddcl_core(lua_State * L){
    luaL_checkversion(L);
    DDLUA_NEWLIB(L, "lddcl.core", _reg);
    return 1;
}
