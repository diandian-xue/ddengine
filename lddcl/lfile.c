#define CLLUA_CORE

#include "lcl.h"
#include "ddclfile.h"

#include <stdlib.h>
#include <stdio.h>

#define ERROR_CHECK(L, E) { if(E){ return luaL_error(L, ddcl_err(E)); } }

static int
l_full_path(lua_State * L){
    const char * path = luaL_checkstring (L, 1);
    char out[DDCL_MAX_PATH];
    ERROR_CHECK(L, ddcl_full_path(path, out));
    lua_pushstring(L, out);
    return 1;
}

static int
l_cwd(lua_State * L){
    lua_pushstring(L, ddcl_cwd());
    return 1;
}

static int
l_path_absolute(lua_State * L){
    lua_pushboolean(L, ddcl_path_absolute(luaL_checkstring(L, 1)));
    return 1;
}

static int
l_file_exist(lua_State * L){
    lua_pushboolean(L, ddcl_file_exist(luaL_checkstring(L, 1)));
    return 1;
}

static int
l_is_dir(lua_State * L){
    lua_pushboolean(L, ddcl_is_dir(luaL_checkstring(L, 1)));
    return 1;
}

static int
l_file_size(lua_State * L){
    size_t sz;
    ERROR_CHECK(L, ddcl_file_size(luaL_checkstring(L, 1), &sz));
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
l_add_path(lua_State * L){
    ddcl_FileSearch * fs = lua_touserdata(L, 1);
    if(!fs){
        return luaL_error(L, "invalid userdata pointer");
    }
    ERROR_CHECK(L, ddcl_add_path(fs, luaL_checkstring(L, 2)));
    return 0;
}

static int
l_find_file(lua_State * L){
    ddcl_FileSearch * fs = lua_touserdata(L, 1);
    if(!fs){
        return luaL_error(L, "invalid userdata pointer");
    }
    char out[DDCL_MAX_PATH];
    ERROR_CHECK(L, ddcl_find_file(fs, luaL_checkstring(L, 2), out));
    lua_pushstring(L, out);
    return 1;
}

int
openlib_file (lua_State * L){
    DDLUA_PUSHFUNC(L, "full_path", l_full_path);
    DDLUA_PUSHFUNC(L, "cwd", l_cwd);
    DDLUA_PUSHFUNC(L, "path_absolute", l_path_absolute);
    DDLUA_PUSHFUNC(L, "file_exist", l_file_exist);
    DDLUA_PUSHFUNC(L, "is_dir", l_is_dir);
    DDLUA_PUSHFUNC(L, "file_size", l_file_size);
    DDLUA_PUSHFUNC(L, "new_file_search", l_new_file_search);
    DDLUA_PUSHFUNC(L, "add_path", l_add_path);
    DDLUA_PUSHFUNC(L, "find_file", l_find_file);

    return 0;
}

