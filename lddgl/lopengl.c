#define DDGLLUA_CORE

#include "ddgl.h"
#include "ddglmath.h"

#include "lgl.h"

#include <GL/glew.h>
#include <stdlib.h>
#include <memory.h>


static int
l_glGenBuffers(lua_State * L){
    int count = luaL_checkinteger(L, 1);
    lua_newtable(L);
    GLuint buf;
    for(int i = 1; i <= count; i ++){
        glGenBuffers(1, &buf);
        lua_pushinteger(L, buf);
        lua_seti(L, -2, i);
    }
    return 1;
}

static int
l_glGenTextures(lua_State * L){
    int count = luaL_checkinteger(L, 1);
    lua_newtable(L);
    GLuint tx;
    for(int i = 1; i <= count; i ++){
        glGenTextures(1, &tx);
        lua_pushinteger(L, tx);
        lua_seti(L, -2, i);
    }
    return 1;
}

static int
l_glBindTexture(lua_State * L){
    GLenum em = luaL_checkinteger(L, 1);
    GLuint tx = luaL_checkinteger(L, 2);
    glBindTexture(em, tx);
    return 0;
}

static int
l_glTexImage2D(lua_State * L){
    GLenum em = luaL_checkinteger(L, 1);
    int lvl = luaL_checkinteger(L, 2);
    GLenum format_em = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    int border = 0;
    GLenum data_format = luaL_checkinteger(L, 6);
    GLenum data_em = luaL_checkinteger(L, 7);
    void * data = NULL;
    switch(lua_type(L, 8)){
    case LUA_TUSERDATA:
        data = lua_touserdata(L, 8);
        break;
    case LUA_TLIGHTUSERDATA:
        data = lua_touserdata(L, 8);
        break;
    case LUA_TSTRING:
        data = lua_tostring(L, 8);
        break;
    default:
        return luaL_error(L, "#8 error %s", lua_typename(L, lua_type(L, 8)));
    }
    if(!data){
        return luaL_error(L, "image data is null");
    }
    glTexImage2D(em, lvl, format_em, width, height, border, data_format, data_em, data);
    return 0;
}

static int
l_glTexParameteri(lua_State * L){
    GLenum tx_em = luaL_checkinteger(L, 1);
    GLenum type_em = luaL_checkinteger(L, 2);
    GLenum value_em = luaL_checkinteger(L, 3);

    glTexParameteri(tx_em, type_em, value_em);
    return 0;
}

static int
l_glTexParameteriv(lua_State * L){
    GLenum tx_em = luaL_checkinteger(L, 1);
    GLenum type_em = luaL_checkinteger(L, 2);

    luaL_checktype(L, 3, LUA_TTABLE);
    lua_len(L, 3);
    int len = lua_tointeger(L, -1);
    GLenum * sw = malloc(sizeof(GLenum) * len);
    for(int i = 0; i < len; i ++){
        lua_geti(L, 3, i + 1);
        sw[i] = luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }
    glTexParameteriv(tx_em, type_em, sw);
    free(sw);
    return 0;
}

static int
l_glBindBuffer(lua_State * L){
    int em = (int)luaL_checkinteger(L, 1);
    GLuint buf = (GLuint)luaL_checkinteger(L, 2);
    glBindBuffer(em, buf);
    return 0;
}

static int
l_glGenVertexArrays(lua_State * L){
    int count = luaL_checkinteger(L, 1);
    lua_newtable(L);
    for (int i = 1; i <= count; i ++){
        GLuint obj;
        glGenVertexArrays(1, &obj);
        lua_pushinteger(L, obj);
        lua_seti(L, -2, i);
    }
    return 1;
}

static int
l_glBindVertexArray(lua_State * L){
    GLuint VAO = luaL_checkinteger(L, 1);
    glBindVertexArray(VAO);
    return 0;
}

static int
l_glBufferData(lua_State * L){
    GLenum obj_em = luaL_checkinteger(L, 1);
    GLenum type_em = luaL_checkinteger(L, 2);
    size_t sz;

	struct {
		struct {
			float x, y, z;
			unsigned char r, g, b, a;
		}p[4];
	} * data = NULL;
    //const void * data = NULL;
    switch(lua_type(L, 3)){
    case LUA_TLIGHTUSERDATA:
    case LUA_TUSERDATA:
        data = lua_touserdata(L, 3);
        sz = luaL_checkinteger(L, 4);
        break;
    case LUA_TSTRING:
        data = lua_tolstring(L, 3, &sz);
        break;
    default:
        return luaL_error(L, "param #3 expected userdata or string");
    }
    if(!data){
        return luaL_error(L, "data is NULL");
    }
    glBufferData(obj_em, sz, data, type_em);
    return 0;
}

static int
l_glVertexAttribPointer(lua_State * L){
    int location = luaL_checkinteger(L, 1);
    int em_count = luaL_checkinteger(L, 2);
    GLenum em_format = luaL_checkinteger(L, 3);
    int normalized  = lua_toboolean(L, 4);
    int stride = luaL_checkinteger(L, 5);
    int pointer = luaL_checkinteger(L, 6);
    glVertexAttribPointer(location, em_count, em_format,
            normalized, stride, (void *)pointer);
    return 0;
}

static int
l_glEnableVertexAttribArray(lua_State * L){
    int location = luaL_checkinteger(L, 1);
    glEnableVertexAttribArray(location);
    return 0;
}

static int
l_glBufferData2(lua_State * L){
    float vertices[] = {
        // positions          // colors           // texture coords
         400.5f,  200.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f,   // top right
         400.5f, -200.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,   // bottom right
        -400.5f, -200.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,   // bottom left
        -400.5f,  200.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 0.0f    // top left 
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    return 0;
}

static int
l_glCreateProgram(lua_State * L){
    const char * vert_s = lua_tostring(L, 1);
    const char * frag_s = lua_tostring(L, 2);

    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vert_s, NULL);
    glCompileShader(vert);

    int success;
    char infolog[512];
    glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(vert, 512, NULL, infolog);
        glDeleteShader(vert);
        return luaL_error(L, "vert error:%s", infolog);
    }

    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &frag_s, NULL);
    glCompileShader(frag);
    // check for shader compile errors
    glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(frag, 512, NULL, infolog);
        glDeleteShader(frag);
        return luaL_error(L, "frag error:%s", infolog);
    }
    // link shaders
    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);
    // check for linking errors
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glDeleteShader(vert);
        glDeleteShader(frag);
        glGetProgramInfoLog(program, 512, NULL, infolog);
        return luaL_error(L, "link error:%s", infolog);
    }
    glDeleteShader(vert);
    glDeleteShader(frag);

    glUseProgram(program);
    lua_pushinteger(L, program);
    return 1;
}

static int
l_glUseProgram(lua_State * L){
    GLuint program = luaL_checkinteger(L, 1);
    glUseProgram(program);
    return 0;
}

static int
l_glClear(lua_State * L){
    glDepthMask(GL_TRUE);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthMask(GL_FALSE);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glAlphaFunc(GL_GREATER, 0.01f);
    glEnable(GL_ALPHA_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    return 0;
}

static int
l_glDrawArrays(lua_State * L){
    //glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    return 0;
}

static int
l_glDrawElements(lua_State * L){
    GLenum em = luaL_checkinteger(L, 1);
    int count = luaL_checkinteger(L, 2);
    GLenum format_em = luaL_checkinteger(L, 3);
    int indices = luaL_checkinteger(L, 4);
    glDrawElements(em, count, format_em, (void*) indices);

    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
    return 0;
}

static int
l_glUniform1f(lua_State * L){
    GLuint program = luaL_checkinteger(L, 1);
    const char * name = luaL_checkstring(L, 2);
    float f = luaL_checknumber(L, 3);

    glUseProgram(program);
    int loc = glGetUniformLocation(program, name);
    if(loc < 0){
        return luaL_error(L,
                "not founded uniform: %s code:%d err:%d",
                name, loc, glGetError());
    }
    glUniform1f(loc, f);
    //glUseProgram(0);
    lua_pushinteger(L, loc);
    return 1;
}

static int
l_glUniformMatrix(lua_State * L){
    GLuint program = luaL_checkinteger(L, 1);
    const char * name = luaL_checkstring(L, 2);

    luaL_checktype(L, 3, LUA_TUSERDATA);
    ddgl_Matrix * m = lua_touserdata(L, 3);

    int loc = glGetUniformLocation(program, name);
    if(loc < 0){
        return luaL_error(L,
                "not founded uniform: %s code:%d err:%d",
                name, loc, glGetError());
    }
    GLfloat glm[16];
    for(int i = 0; i < 16; i ++){
        glm[i] = m->A[i];
    }
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm);
    lua_pushinteger(L, loc);
    return 1;
}

static int
l_glGetError(lua_State * L){
    lua_pushinteger(L, glGetError());
    return 1;
}

static int
l_sizeof(lua_State * L){
    GLenum em = luaL_checkinteger(L, 1);
    size_t sz;
    switch(em){
    case GL_UNSIGNED_INT:
        sz = sizeof(GLint);
        break;
    case GL_FLOAT:
        sz = sizeof(GLfloat);
        break;
    default:
        return luaL_error(L, "error em type");
    }
    lua_pushinteger(L, sz);
    return 1;
}

int
open_opengl_module(lua_State * L){
    lua_newtable(L);
    lua_pushstring(L, "opengl");
    lua_pushvalue(L, -2);
    lua_rawset(L, -4);

    DDLUA_PUSHFUNC(L, "glGenBuffers", l_glGenBuffers);
    DDLUA_PUSHFUNC(L, "glBufferData", l_glBufferData);
    DDLUA_PUSHFUNC(L, "glBindBuffer", l_glBindBuffer);
    DDLUA_PUSHFUNC(L, "glGenVertexArrays", l_glGenVertexArrays);
    DDLUA_PUSHFUNC(L, "glBindVertexArray", l_glBindVertexArray);
    DDLUA_PUSHFUNC(L, "glVertexAttribPointer", l_glVertexAttribPointer);
    DDLUA_PUSHFUNC(L, "glEnableVertexAttribArray", l_glEnableVertexAttribArray);

    DDLUA_PUSHFUNC(L, "glGenTextures", l_glGenTextures);
    DDLUA_PUSHFUNC(L, "glBindTexture", l_glBindTexture);
    DDLUA_PUSHFUNC(L, "glTexImage2D", l_glTexImage2D);
    DDLUA_PUSHFUNC(L, "glTexParameteri", l_glTexParameteri);
    DDLUA_PUSHFUNC(L, "glTexParameteriv", l_glTexParameteriv);

    DDLUA_PUSHFUNC(L, "glCreateProgram", l_glCreateProgram);
    DDLUA_PUSHFUNC(L, "glUseProgram", l_glUseProgram);
    DDLUA_PUSHFUNC(L, "glClear", l_glClear);
    DDLUA_PUSHFUNC(L, "glDrawArrays", l_glDrawArrays);
    DDLUA_PUSHFUNC(L, "glDrawElements", l_glDrawElements);
    DDLUA_PUSHFUNC(L, "glUniformMatrix", l_glUniformMatrix);
    DDLUA_PUSHFUNC(L, "glUniform1f", l_glUniform1f);
    DDLUA_PUSHFUNC(L, "glGetError", l_glGetError);

    DDLUA_PUSHFUNC(L, "sizeof", l_sizeof);

    DDLUA_PUSHENUM(L, "GL_ARRAY_BUFFER", GL_ARRAY_BUFFER);
    DDLUA_PUSHENUM(L, "GL_ELEMENT_ARRAY_BUFFER", GL_ARRAY_BUFFER);

    DDLUA_PUSHENUM(L, "GL_TEXTURE_1D", GL_TEXTURE_1D);
    DDLUA_PUSHENUM(L, "GL_TEXTURE_2D", GL_TEXTURE_2D);
    DDLUA_PUSHENUM(L, "GL_TEXTURE_3D", GL_TEXTURE_3D);
    DDLUA_PUSHENUM(L, "GL_TEXTURE_WRAP_S", GL_TEXTURE_WRAP_S);
    DDLUA_PUSHENUM(L, "GL_TEXTURE_WRAP_T", GL_TEXTURE_WRAP_T);
    DDLUA_PUSHENUM(L, "GL_TEXTURE_MIN_FILTER", GL_TEXTURE_MIN_FILTER);
    DDLUA_PUSHENUM(L, "GL_TEXTURE_MAG_FILTER", GL_TEXTURE_MAG_FILTER);
    DDLUA_PUSHENUM(L, "GL_TEXTURE_SWIZZLE_RGBA", GL_TEXTURE_SWIZZLE_RGBA);

    DDLUA_PUSHENUM(L, "GL_REPEAT", GL_REPEAT);
    DDLUA_PUSHENUM(L, "GL_MIRRORED_REPEAT", GL_MIRRORED_REPEAT);
    DDLUA_PUSHENUM(L, "GL_CLAMP_TO_EDGE", GL_CLAMP_TO_EDGE);
    DDLUA_PUSHENUM(L, "GL_CLAMP_TO_BORDER", GL_CLAMP_TO_BORDER);
    DDLUA_PUSHENUM(L, "GL_LINEAR", GL_LINEAR);
    DDLUA_PUSHENUM(L, "GL_NEAREST", GL_NEAREST);

    DDLUA_PUSHENUM(L, "GL_RED", GL_RED);
    DDLUA_PUSHENUM(L, "GL_GREEN", GL_GREEN);
    DDLUA_PUSHENUM(L, "GL_BLUE", GL_BLUE);
    DDLUA_PUSHENUM(L, "GL_ALPHA", GL_ALPHA);
    DDLUA_PUSHENUM(L, "GL_RGBA8", GL_RGBA8);
    DDLUA_PUSHENUM(L, "GL_RGBA", GL_RGBA);
    DDLUA_PUSHENUM(L, "GL_ONE", GL_ONE);

    DDLUA_PUSHENUM(L, "GL_UNSIGNED_BYTE", GL_UNSIGNED_BYTE);
    DDLUA_PUSHENUM(L, "GL_FLOAT", GL_FLOAT);

    DDLUA_PUSHENUM(L, "GL_STATIC_DRAW", GL_STATIC_DRAW);


    DDLUA_PUSHENUM(L, "GL_TRIANGLES", GL_TRIANGLES);

    lua_pop(L, 1);
    return 0;
}

