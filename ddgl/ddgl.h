#pragma once

#include "ddglconfig.h"

enum ddgl_Engine {
    DDGL_GDI = 1,
    DDGL_OPENGL,
    DDGL_DX11,
    DDGL_DX12,
    DDGL_VULKAN,
    DDGL_METAL,
};

typedef struct tag_ddgl{
    enum ddgl_Engine engine;
}ddgl;

DDGLAPI int
ddgl_init(ddgl * conf);

DDGLAPI int
ddgl_exit();

DDGLAPI void
ddgl_default(ddgl * conf);

DDGLAPI void
ddgl_init_modules();

DDGLAPI ddgl *
ddgl_conf();
