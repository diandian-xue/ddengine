#pragma once

#include "ddgl.h"
#include "ddclstorage.h"

DDGLAPI ddcl_Handle DDGL_OPENGL_SHAPE2D_SHADER;
DDGLAPI ddcl_Handle DDGL_OPENGL_SHAPE2D_MATERIAL;
DDGLAPI ddcl_Handle DDGL_OPENGL_SHAPE2D_RENDER;

extern const char DDGL_OPENGL_SHAPE2D_SHADER_VERTS[];
extern const char DDGL_OPENGL_SHAPE2D_SHADER_FRAGS[];

DDGLAPI int
ddgl_init_opengl_shape2d_module(ddgl * conf);

DDGLAPI void
ddgl_exit_opengl_shape2d_module();
