#pragma once

#include "ddgl.h"
#include "ddglshader.h"

#include <GL/glew.h>

#define DDGL_OPENGL_SHADER_UNIFORM_MAX_COUNT 30
#define DDGL_OPENGL_SHADER_ATTR_MAX_COUNT 30


#define DDGL_OPENGL_ABO_BUF_OFFSET(C, B) &((C *)(void*)0)->B

extern const char DDGL_OPENGL_SHADER_VERTS_PRES[];
extern const char DDGL_OPENGL_SHADER_FRAGS_PRES[];

typedef struct tag_ddgl_OpenglShaderVar{
    const char * name;
    GLuint location;
}ddgl_OpenglShaderVar;

typedef struct tag_ddgl_OpenglShader{
    GLuint program;

    GLuint uniforms_count;
    ddgl_OpenglShaderVar uniforms[DDGL_OPENGL_SHADER_UNIFORM_MAX_COUNT];

    GLuint attr_count;
    ddgl_OpenglShaderVar attrs[DDGL_OPENGL_SHADER_ATTR_MAX_COUNT];
}ddgl_OpenglShader;

typedef struct tag_ddgl_GenOpenglShader{
    const char * verts[20];
    int vcount;
    const char * frags[20];
    int fcount;

    ddgl_OpenglShaderVar * uniforms;
    ddgl_OpenglShaderVar attrs;
}ddgl_GenOpenglShader;

DDGLAPI int
ddgl_gen_opengl_shader(ddgl_Shader * shader, ddgl_GenOpenglShader * gen);

DDGLAPI void
ddgl_set_opengl_vbo_vertex_attrib();
