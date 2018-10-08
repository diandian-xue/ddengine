#pragma once

#include "ddgl.h"
#include "ddclstorage.h"

DDGLAPI ddcl_Handle DDGL_OPENGL_IMAGE2D_SHADER;
DDGLAPI ddcl_Handle DDGL_OPENGL_IMAGE2D_MATERIAL;
DDGLAPI ddcl_Handle DDGL_OPENGL_IMAGE2D_RENDER;

DDGLAPI const char DDGL_OPENGL_IMAGE2D_SHADER_VERTS[];
DDGLAPI const char DDGL_OPENGL_IMAGE2D_SHADER_FRAGS[];

DDGLAPI int
ddgl_init_opengl_image2d_module(ddgl * conf);

DDGLAPI void
ddgl_exit_opengl_image2d_module();
