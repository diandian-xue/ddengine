#pragma once

#include "ddgl.h"
#include "ddglmath.h"

#include "ddclstorage.h"

enum ddgl_ShaderType{
    DDGL_VERTEX_SHADER  = 1,
    DDGL_FRAGMENT_SHADER,
};

#define DDGL_SHADER_EXTEND 512

typedef struct tag_ddgl_Shader{
    int ref;
    ddcl_Handle h;
    char extend[DDGL_SHADER_EXTEND];
    void(*free)(struct tag_ddgl_Shader *);
    void(*use)(struct tag_ddgl_Shader *);
    void(*set_uniform_mtrix)(
            struct tag_ddgl_Shader *, const char * name, ddgl_Matrix * m);
}ddgl_Shader;

typedef struct tag_ddgl_ShaderScript{
    enum ddgl_ShaderType type;
    const char * script;
}ddgl_ShaderScript;

DDGLAPI void
ddgl_init_shader_module(ddgl * conf);

DDGLAPI void
ddgl_exit_shader_module();

DDGLAPI ddgl_Shader *
ddgl_new_shader();

DDGLAPI ddgl_Shader *
ddgl_retain_shader(ddcl_Handle h);

DDGLAPI inline void
ddgl_release_shader(ddcl_Handle h);

DDGLAPI void
ddgl_set_shader_uniform_matrix(const char * name, ddgl_Matrix * m);
