#pragma once

#include "ddgl.h"
#include "ddglshader.h"

#include <GL/glew.h>

DDGLAPI int
ddgl_init_opengl_material_module(ddgl * conf);

DDGLAPI int
ddgl_exit_opengl_material_module();
