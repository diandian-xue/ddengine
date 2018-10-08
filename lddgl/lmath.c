#define DDGLLUA_CORE

#include "ddgl.h"
#include "lgl.h"
#include "ddglmath.h"

static int
l_new_vec(lua_State * L){
    float * vec = lua_newuserdata(L, sizeof(ddgl_Vec));
    if(lua_gettop(L) > 1){
        switch(lua_type(L, 1)){
        case LUA_TTABLE:
            memset(vec, 0, sizeof(ddgl_Vec));
            luaL_checktype(L, 1, LUA_TTABLE);
            lua_len(L, 1);
            int len = lua_tointeger(L, -1);
            len = len > 4 ? 4 : len;
            lua_pop(L, 1);

            for(int i = 0; i < len; i ++){
                lua_geti(L, 1, i + 1);
                float v = luaL_checknumber(L, -1);
                vec[i] = v;
                lua_pop(L, 1);
            }
            break;
        case LUA_TUSERDATA:
            memcpy(vec, lua_touserdata(L, 1), sizeof(ddgl_Vec));
            break;
        case LUA_TNUMBER:
            int top = lua_gettop(L);
            top = top > 4 ? 4 : top;
            for (int i = 1; i <= top; i ++){
                vec[i - 1] = luaL_checknumber(L, i);
            }
            break;
        default:
            return luaL_error(L, "bad argument #1, table or userdata expected, got %s",
                    lua_typename(L, lua_type(L, 1)));
        }
    }else{
        memset(vec, 0, sizeof(ddgl_Vec));
    }
    return 1;
}

static int
l_table_vec(lua_State * L){
    luaL_checktype(L, 1, LUA_TUSERDATA);
    ddgl_Vec * vec = lua_touserdata(L, 1);
    lua_newtable(L);
    for (int i = 0; i < 4; i ++){
        lua_pushnumber(L, *(&(vec->x) + i));
        lua_seti(L, -2, i + 1);
    }
    return 1;
}

static int
l_length_vec(lua_State * L){
    luaL_checktype(L, 1, LUA_TUSERDATA);
    ddgl_Vec * vec = lua_touserdata(L, 1);
    lua_pushnumber(L, ddgl_length_vec(vec));
    return 1;
}

static int
l_normalize_vec(lua_State * L){
    luaL_checktype(L, 1, LUA_TUSERDATA);
    ddgl_Vec * vec = lua_touserdata(L, 1);
    ddgl_normalize_vec(vec);
    return 0;
}

static int
l_negation_vec(lua_State * L){
    luaL_checktype(L, 1, LUA_TUSERDATA);
    ddgl_Vec * vec = lua_touserdata(L, 1);
    ddgl_negation_vec(vec);
    return 0;
}

static int
l_addition_vec(lua_State * L){
    luaL_checktype(L, 1, LUA_TUSERDATA);
    luaL_checktype(L, 2, LUA_TUSERDATA);
    ddgl_Vec * v1 = lua_touserdata(L, 1);
    ddgl_Vec * v2 = lua_touserdata(L, 2);
    ddgl_addition_vec(v1, v2);
    return 0;
}

static int
l_dot_product_vec(lua_State * L){
    luaL_checktype(L, 1, LUA_TUSERDATA);
    luaL_checktype(L, 2, LUA_TUSERDATA);
    ddgl_Vec * v1 = lua_touserdata(L, 1);
    ddgl_Vec * v2 = lua_touserdata(L, 2);
    lua_pushnumber(L, ddgl_dot_product_vec(v1, v2));
    return 1;
}

static int
l_cross_product_vec(lua_State * L){
    luaL_checktype(L, 1, LUA_TUSERDATA);
    luaL_checktype(L, 2, LUA_TUSERDATA);
    ddgl_Vec * v1 = lua_touserdata(L, 1);
    ddgl_Vec * v2 = lua_touserdata(L, 2);
    ddgl_cross_product_vec(v1, v2);
    return 0;
}

static int
l_product_vec_scalar(lua_State * L){
    luaL_checktype(L, 1, LUA_TUSERDATA);
    float scalar = luaL_checknumber(L, 2);
    ddgl_Vec * v = lua_touserdata(L, 1);
    ddgl_product_vec_scalar(v, scalar);
    return 0;
}

static int
l_product_vec_matrix(lua_State * L){
    luaL_checktype(L, 1, LUA_TUSERDATA);
    luaL_checktype(L, 2, LUA_TUSERDATA);
    ddgl_Vec * v = lua_touserdata(L, 1);
    ddgl_Matrix * m = lua_touserdata(L, 2);
    ddgl_product_vec_matrix(v, m);
    return 0;
}


static int
l_new_matrix(lua_State * L){
    ddgl_Matrix * m = lua_newuserdata(L, sizeof(ddgl_Matrix));
    if (lua_gettop(L) > 1){
        switch(lua_type(L, 1)){
        case LUA_TTABLE:
            lua_len(L, 1);
            int len = lua_tointeger(L, -1);
            len = len > 16 ? 16 : len;
            lua_pop(L, 1);

            memset(m, 0, sizeof(ddgl_Matrix));
            for (int i = 1; i <= len; i ++){
                lua_geti(L, 1, i);
                float v = luaL_checknumber(L, -1);
                m->A[i - 1] = v;
                lua_pop(L, 1);
            }
            break;
        case LUA_TUSERDATA:
            memcpy(m, lua_touserdata(L, 1), sizeof(ddgl_Matrix));
            break;
        default:
            return luaL_error(L, "bad argument #1, table or userdata expected, got %s",
                    lua_typename(L, lua_type(L, 1)));
        }
    }else{
        ddgl_identity_matrix(m);
    }
    return 1;
}

static int
l_table_matrix(lua_State * L){
    luaL_checktype(L, 1, LUA_TUSERDATA);
    ddgl_Matrix * m = lua_touserdata(L, 1);
    if(!m){
        return luaL_error(L, "(ddgl_Matrix *) pointer, got null");
    }

    lua_newtable(L);
    for (int i = 1; i <= 16; i ++){
        lua_pushnumber(L, m->A[i - 1]);
        lua_seti(L, -2, i);
    }
    return 1;
}

static int
l_product_matrix(lua_State * L){
    luaL_checktype(L, 1, LUA_TUSERDATA);
    luaL_checktype(L, 2, LUA_TUSERDATA);
    ddgl_Matrix * m1 = lua_touserdata(L, 1);
    ddgl_Matrix * m2 = lua_touserdata(L, 2);
    ddgl_product_matrix(m1, m2);
    return 0;
}

static int
l_identity_matrix(lua_State * L){
    luaL_checktype(L, 1, LUA_TUSERDATA);
    ddgl_Matrix * m = lua_touserdata(L, 1);
    ddgl_identity_matrix(m);
    return 0;
}

static int
l_addition_matrix(lua_State * L){
    luaL_checktype(L, 1, LUA_TUSERDATA);
    ddgl_Matrix * m = lua_touserdata(L, 1);
    float scalar = luaL_checknumber(L, 2);
    ddgl_addition_matrix(m, scalar);
    return 0;
}

static int
l_product_matrix_scalar(lua_State * L){
    luaL_checktype(L, 1, LUA_TUSERDATA);
    float scalar = luaL_checknumber(L, 2);
    ddgl_Matrix * m = lua_touserdata(L, 1);
    ddgl_product_matrix_scalar(m, scalar);
    return 0;
}

static int
l_translate_matrix(lua_State * L){
    luaL_checktype(L, 1, LUA_TUSERDATA);
    ddgl_Matrix * m = lua_touserdata(L, 1);

    float def[3] = { 0.0f, 0.0f, 0.0f };
    switch(lua_type(L, 2)){
    case LUA_TTABLE:
        lua_len(L, 2);
        int len = lua_tointeger(L, 2);
        len = len > 3 ? 3 : len;
        for (int i = 1; i <= len; i ++){
            lua_geti(L, 2, i);
            def[i - 1] = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        break;
    case LUA_TNUMBER:
        int top = lua_gettop(L);
        top = top > 4 ? 4 : top;
        for(int i = 2; i <= top; i ++){
            def[i - 2] = luaL_checknumber(L, i);
        }
        break;
    case LUA_TUSERDATA:
        ddgl_Vec * v = lua_touserdata(L, 2);
        def[0] = v->x; def[1] = v->y; def[2] = v->z;
        break;
    default:
        return luaL_error(L, "bad argument #2, table or number expected, got %s",
                lua_typename(L, lua_type(L, 1)));
    }
    ddgl_translate_matrix(m, def[0], def[1], def[2]);
    return 0;
}

static int
l_scale_matrix(lua_State * L){
    luaL_checktype(L, 1, LUA_TUSERDATA);
    ddgl_Matrix * m = lua_touserdata(L, 1);
    float def[3] = { 1.0f, 1.0f, 1.0f };
    switch(lua_type(L, 2)){
    case LUA_TTABLE:
        lua_len(L, 2);
        int len = lua_tointeger(L, 2);
        len = len > 3 ? 3 : len;
        for (int i = 1; i <= len; i ++){
            lua_geti(L, 2, i);
            def[i - 1] = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
        break;
    case LUA_TNUMBER:
        int top = lua_gettop(L);
        top = top > 4 ? 4 : top;
        for(int i = 2; i <= top; i ++){
            def[i - 2] = luaL_checknumber(L, i);
        }
        break;
    case LUA_TUSERDATA:
        ddgl_Vec * v = lua_touserdata(L, 2);
        def[0] = v->x; def[1] = v->y; def[2] = v->z;
        break;
    default:
        return luaL_error(L, "bad argument #2, table or number expected, got %s",
                lua_typename(L, lua_type(L, 1)));
    }
    ddgl_scale_matrix(m, def[0], def[1], def[2]);
    return 0;
}

static int
l_rotate_x_matrix(lua_State * L){
    luaL_checktype(L, 1, LUA_TUSERDATA);
    ddgl_Matrix * m = lua_touserdata(L, 1);
    float angle = luaL_checknumber(L, 2);
    ddgl_rotate_x_matrix(m, angle);
    return 0;
}

static int
l_rotate_y_matrix(lua_State * L){
    luaL_checktype(L, 1, LUA_TUSERDATA);
    ddgl_Matrix * m = lua_touserdata(L, 1);
    float angle = luaL_checknumber(L, 2);
    ddgl_rotate_y_matrix(m, angle);
    return 0;
}

static int
l_rotate_z_matrix(lua_State * L){
    luaL_checktype(L, 1, LUA_TUSERDATA);
    ddgl_Matrix * m = lua_touserdata(L, 1);
    float angle = luaL_checknumber(L, 2);
    ddgl_rotate_z_matrix(m, angle);
    return 0;
}


int
open_math_module(lua_State * L){
    DDLUA_PUSHFUNC(L, "new_vec", l_new_vec);
    DDLUA_PUSHFUNC(L, "table_vec", l_table_vec);
    DDLUA_PUSHFUNC(L, "length_vec", l_length_vec);
    DDLUA_PUSHFUNC(L, "normalize_vec", l_normalize_vec);
    DDLUA_PUSHFUNC(L, "negation_vec", l_negation_vec);
    DDLUA_PUSHFUNC(L, "addition_vec", l_addition_vec);
    DDLUA_PUSHFUNC(L, "dot_product_vec", l_dot_product_vec);
    DDLUA_PUSHFUNC(L, "cross_product_vec", l_cross_product_vec);
    DDLUA_PUSHFUNC(L, "product_vec_scalar", l_product_vec_scalar);
    DDLUA_PUSHFUNC(L, "product_vec_matrix", l_product_vec_matrix);

    DDLUA_PUSHFUNC(L, "new_matrix", l_new_matrix);
    DDLUA_PUSHFUNC(L, "identity_matrix", l_identity_matrix);
    DDLUA_PUSHFUNC(L, "table_matrix", l_table_matrix);
    DDLUA_PUSHFUNC(L, "product_matrix", l_product_matrix);
    DDLUA_PUSHFUNC(L, "translate_matrix", l_translate_matrix);
    DDLUA_PUSHFUNC(L, "scale_matrix", l_scale_matrix);
    DDLUA_PUSHFUNC(L, "rotate_x_matrix", l_rotate_x_matrix);
    DDLUA_PUSHFUNC(L, "rotate_y_matrix", l_rotate_y_matrix);
    DDLUA_PUSHFUNC(L, "rotate_z_matrix", l_rotate_z_matrix);
    return 0;
}
