#pragma once

#include "ddgl.h"
#include "ddglsprite.h"
#include "ddgltexture.h"

DDGLAPI ddcl_Handle DDGL_IMAGE2D_RENDER;

typedef ddgl_Sprite ddgl_Image2d;

DDGLAPI ddgl_Image2d *
ddgl_new_image2d(ddgl_Texture * tex);
