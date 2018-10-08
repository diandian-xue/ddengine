#pragma once

#include "ddgl.h"
#include "ddglshader.h"

#include "ddclstorage.h"

#define DDGL_MATERIAL_EXTEND 256

typedef struct tag_ddgl_Material{
    int ref;
    ddcl_Handle h;
    ddgl_Shader * shader;
    char extend[DDGL_MATERIAL_EXTEND];
    void(*free)(struct tag_ddgl_Material *);
}ddgl_Material;

DDGLAPI int
ddgl_init_material_module(ddgl * conf);

DDGLAPI void
ddgl_exit_meterial_module();

DDGLAPI ddgl_Material *
ddgl_new_material();

DDGLAPI ddgl_Material *
ddgl_retain_material(ddcl_Handle h);

DDGLAPI void
ddgl_releaase_material(ddgl_Material * m);

