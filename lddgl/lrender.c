#define DDGLLUA_CORE

#include "ddgl.h"
#include "lgl.h"
#include "ddglrender.h"

#include <GL/glew.h>


static int
l_render_image(lua_State * L){
    /*
    GLuint texture = luaL_checkinteger(L, 1);

    ddgl_RenderPoint points[4] = {0};
    ddgl_Vec p[4] = {0};
    p[0].x = -0.5f;
    p[0].y = 0.5f;
    p[1].x = -0.5f;
    p[1].y = -0.5f;
    p[2].x = 0.5f;
    p[2].y = 0.5f;
    p[3].x = 0.5f;
    p[4].y = -0.5f;

    ddgl_UV uv[4] = {0};
    uv[0].u = 0.0f;
    uv[0].v = 0.0f;
    uv[1].u = 0.0f;
    uv[1].u = 1.0f;
    uv[2].u = 1.0f;
    uv[2].u = 0.0f;
    uv[3].v = 1.0f;
    uv[3].v = 1.0f;
    for (int i = 0; i < 4; i ++){
        points[i].position = p[i];
        points[i].position.w = 1.0f;
        memset(&(points[i].color), 255, sizeof(ddgl_Color));
        points[i].uv = uv[i];
    }
    ddgl_render_image(points, texture);
     */
    return 0;
}

int
open_render_module(lua_State * L){
    DDLUA_PUSHFUNC(L, "render_image", l_render_image);

    return 0;
}
