#pragma once

#include "ddglconfig.h"
#include "ddglsprite.h"

#include "ddclstorage.h"

DDGLAPI ddcl_Handle DDGL_SHAPE2D_RENDER;

typedef ddgl_Sprite ddgl_Shape2d;

DDGLAPI ddgl_Shape2d *
ddgl_new_shape2d();
