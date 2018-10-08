#pragma once

#include "ddgl.h"
#include "ddgltexture.h"


DDGLAPI int
ddgl_gen_opengl_texture(ddgl_Texture * tex);

DDGLAPI int
ddgl_gen_opengl_texture_with_png(ddgl_Texture * tex, const char * png);

DDGLAPI int
ddgl_gen_opengl_texture_with_utf8map(ddgl_Texture * tex, const char * map);
