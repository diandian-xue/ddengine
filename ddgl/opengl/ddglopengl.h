#pragma once

#include "ddgl.h"
#include "ddglwindow.h"

DDGLAPI int
ddgl_init_opengl_module(ddgl * conf);

DDGLAPI int
ddgl_exit_opengl_module();

DDGLAPI int
ddgl_new_opengl_window(ddgl_Window * w);
