#pragma once

#include "ddclstorage.h"

#include "ddgl.h"
#include "ddglmaterial.h"

#define DDGL_RENDER_EXTEND 256

typedef struct tag_ddgl_Render{
    int ref;
    ddcl_Handle h;
    ddgl_Material * material;
    char extend[DDGL_RENDER_EXTEND];
    void(*draw)(struct tag_ddgl_Render * render,
            ddgl_Vertex * vertex, int count);
    void(*free)(struct tag_ddgl_Render * render);
}ddgl_Render;


DDGLAPI void
ddgl_init_render_module(ddgl * conf);

DDGLAPI void
ddgl_exit_render_module();

DDGLAPI ddgl_Render *
ddgl_new_render(ddgl_Material * material);

DDGLAPI ddgl_Render *
ddgl_retain_render(ddcl_Handle h);

DDGLAPI void
ddgl_release_render(ddgl_Render * render);

