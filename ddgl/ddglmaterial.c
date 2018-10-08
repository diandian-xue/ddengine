#define DDGL_CORE

#include "ddglmaterial.h"

#include "opengl/ddglopengl_material.h"

static ddcl_Storage * _S;

DDGLAPI int
ddgl_init_material_module(ddgl * conf){
    _S = ddcl_new_storage(sizeof(ddgl_Material), 32);
    return 0;
}

DDGLAPI void
ddgl_exit_meterial_module(){
    ddcl_free_storage(_S);
}

DDGLAPI ddgl_Material *
ddgl_new_material(){
    ddgl_Material * material;
    ddcl_Handle h = ddcl_register_in_storage(_S, &material);
    material->h = h;
    material->ref = 1;
    return material;
}

DDGLAPI ddgl_Material *
ddgl_retain_material(ddcl_Handle h){
    ddgl_Material * material = ddcl_find_in_storage(_S, h);
    if(material){
        material->ref ++;
    }
    return material;
}

DDGLAPI inline void
ddgl_releaase_material(ddgl_Material * material){
    material->ref --;
    if (material->ref <= 0){
        if(material->free){
            material->free(material);
        }
        ddcl_del_in_storage(_S, material->h);
    }
}

