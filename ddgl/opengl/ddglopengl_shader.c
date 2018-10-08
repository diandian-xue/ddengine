#define DDGL_CORE

#include "opengl/ddglopengl_shader.h"
#include "ddglmath.h"

#include <stdlib.h>
#include <stdio.h>

extern const char DDGL_OPENGL_SHADER_VERTS_PRES[] = " \n\
    #version 330 core \n\
    layout (location = 0) in vec4 position; \n\
    layout (location = 1) in vec4 color; \n\
    layout (location = 2) in vec2 uv; \n\
\n\
    uniform mat4 DD_MODEL; \n\
    uniform mat4 DD_CAMERA; \n\
    uniform mat4 DD_PROJECTION; \n\
";

extern const char DDGL_OPENGL_SHADER_FRAGS_PRES[] = " \n\
    #version 330 core \n\
";

static void
opengl_shader_free(ddgl_Shader * shader){
    ddgl_OpenglShader * extend = (ddgl_OpenglShader *)shader->extend;
    glDeleteProgram(extend->program);
}

static void
opengl_shader_use(ddgl_Shader * shader){
    ddgl_OpenglShader * extend = (ddgl_OpenglShader *)shader->extend;
    glUseProgram(extend->program);
}

static void
opengl_shader_set_uniform_matrix(
        ddgl_Shader * shader, const char * name, ddgl_Matrix * m){
    ddgl_OpenglShader * extend = (ddgl_OpenglShader *)shader->extend;
    glUseProgram(extend->program);
    int loc = glGetUniformLocation(extend->program, name);
    if(loc < 0){
        return;
    }
    glUniformMatrix4fv(loc, 1, GL_FALSE, m);
}

DDGLAPI int
ddgl_gen_opengl_shader(ddgl_Shader * shader, ddgl_GenOpenglShader * gen){
    char info[512];
    GLsizei infolen = 512;
    int pos = 0;
    GLint ret;
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    char * verts_r[10];
    verts_r[0] = DDGL_OPENGL_SHADER_VERTS_PRES;
    printf(DDGL_OPENGL_SHADER_VERTS_PRES);
    for(int i = 1; i <= gen->vcount; i ++){
        verts_r[i] = gen->verts[i - 1];
        printf(verts_r[i]);
    }
    glShaderSource(vs, gen->vcount + 1, verts_r, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &ret);
    if (!ret){
        glGetShaderInfoLog(vs, infolen, NULL, info);
        glDeleteShader(vs);
        printf("gen vertex shader error:%s\n", info);
        return 1;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    char * frags_r[10];
    frags_r[0] = DDGL_OPENGL_SHADER_FRAGS_PRES;
    printf(DDGL_OPENGL_SHADER_FRAGS_PRES);
    for(int i = 1; i <= gen->fcount; i ++){
        frags_r[i] = gen->frags[i - 1];
        printf(frags_r[i]);
    }
    glShaderSource(fs, gen->fcount + 1, frags_r, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &ret);
    if (!ret){
        glGetShaderInfoLog(fs, infolen, NULL, info);
        glDeleteShader(fs);
        printf("gen fragment shader error:%s\n", info);
        return 1;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &ret);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!ret){
        glGetProgramInfoLog(program, infolen, NULL, info);
        glDeleteProgram(program);
        printf("gen program shader error:%s\n", info);
        return 1;
    }

    ddgl_OpenglShader * extend = (ddgl_OpenglShader *)shader->extend;
    extend->program = program;
    shader->free = opengl_shader_free;
    shader->use = opengl_shader_use;
    shader->set_uniform_mtrix = opengl_shader_set_uniform_matrix;
    return 0;
}

DDGLAPI void
ddgl_set_opengl_vbo_vertex_attrib(){
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
        sizeof(ddgl_Vertex), DDGL_OPENGL_ABO_BUF_OFFSET(ddgl_Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE,
        sizeof(ddgl_Vertex), DDGL_OPENGL_ABO_BUF_OFFSET(ddgl_Vertex, color));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
        sizeof(ddgl_Vertex), DDGL_OPENGL_ABO_BUF_OFFSET(ddgl_Vertex, uv));
    glEnableVertexAttribArray(2);
}
