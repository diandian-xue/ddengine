#define CLLUA_CORE

#include "lcl.h"
#include "ddclfile.h"

#include <stdlib.h>
#include <stdio.h>

static int
file_err(lua_State * L, int err){
    if (err == 0)
        return 0;
    return 1;
}

static int
l_full_path(lua_State * L){
    const char * path = luaL_checkstring (L, 1);
    char out[DDCL_MAX_PATH];
    file_err(L, ddcl_full_path(path, out));
    lua_pushstring(L, out);
    return 1;
}

static int
l_get_cwd(lua_State * L){
    const char * path = ddcl_cwd();
    lua_pushstring(L, path);
    return 1;
}

static int
l_is_absolute_path(lua_State * L){
    const char * path = luaL_checkstring(L, 1);
    int b = ddcl_path_absolute(path);
    lua_pushboolean(L, b);
    return 1;
}

static int
l_file_is_exist(lua_State * L){
    const char * path = luaL_checkstring(L, 1);
    int b = ddcl_file_exist(path);
    lua_pushboolean(L, b);
    return 1;
}

static int
l_is_dir(lua_State * L){
    const char * path = luaL_checkstring(L, 1);
    int b = ddcl_is_dir(path);
    lua_pushboolean(L, b);
    return 1;
}

static int
l_get_file_size(lua_State * L){
    const char * path = luaL_checkstring(L, 1);
    size_t sz;
    int err = ddcl_file_size(path, &sz);
    file_err(L, err);
    lua_pushinteger(L, sz);
    return 1;
}

static int
l_new_file_search(lua_State * L){
    ddcl_FileSearch * fs = lua_newuserdata(L, sizeof(ddcl_FileSearch));
    ddcl_init_file_search(fs);
    return 1;
}

static int
l_add_in_file_search(lua_State * L){
    ddcl_FileSearch * fs = lua_touserdata(L, 1);
    const char * path = luaL_checkstring(L, 2);
    int err = ddcl_add_path(fs, path);
    file_err(L, err);
    return 0;
}

static int
l_find_in_file_search(lua_State * L){
    ddcl_FileSearch * fs = lua_touserdata(L, 1);
    const char * path = luaL_checkstring(L, 2);
    char out[DDCL_MAX_PATH];
    int err = ddcl_find_file(fs, path, out);
    file_err(L, err);
    lua_pushstring(L, out);
    return 1;
}

static luaL_Reg _REG[] = {
    { "full_path", l_full_path },
    { "get_cwd", l_get_cwd },
    { "is_absolute_path", l_is_absolute_path },
    { "file_is_exist", l_file_is_exist },
    { "is_dir", l_is_dir },
    { "get_file_size", l_get_file_size },
    { "new_file_search", l_new_file_search },
    { "add_in_file_search", l_add_in_file_search },
    { "find_in_file_search", l_find_in_file_search },
    { NULL, NULL },
};

CLLUA int
luaopen_lddcl_file (lua_State * L){
    luaL_checkversion(L);
    DDLUA_NEWLIB(L, "lddcl.file", _REG);
    return 1;
}

