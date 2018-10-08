#define DDGL_CORE


#include "ddglshader.h"
#include "opengl/ddglopengl_shader.h"

static ddcl_Storage * _S;

DDGLAPI void
ddgl_init_shader_module(ddgl * conf){
    _S = ddcl_new_storage(sizeof(ddgl_Shader), 64);
}

DDGLAPI void
ddgl_exit_shader_module(){
    ddcl_free_storage(_S);
}

DDGLAPI ddgl_Shader *
ddgl_new_shader(){
    ddgl_Shader * shader;
    ddcl_Handle h = ddcl_register_in_storage(_S, &shader);
    shader->h = h;
    shader->ref = 1;
    return shader;
}

DDGLAPI ddgl_Shader *
ddgl_retain_shader(ddcl_Handle h){
    ddgl_Shader * shader = ddcl_find_in_storage(_S, h);
    if(shader){
        shader->ref ++;
    }
    return shader;
}

DDGLAPI void
ddgl_release_shader(ddcl_Handle h){
    ddgl_Shader * shader = ddcl_find_in_storage(_S, h);
    if(!shader){
        return;
    }
    shader->ref --;
    if(shader->ref <= 0){
        if(shader->free){
            shader->free(shader);
        }
        ddcl_del_in_storage(_S, shader->h);
    }
    return;
}

DDGLAPI void
ddgl_set_shader_uniform_matrix(const char * name, ddgl_Matrix * m){
    ddcl_begin_storage(_S);
    ddgl_Shader * shader;
    ddcl_Handle h;
    while(ddcl_next_storage(_S, &h, &shader)){
        shader->set_uniform_mtrix(shader, name, m);
    }
}
