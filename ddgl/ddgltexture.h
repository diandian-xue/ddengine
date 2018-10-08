#pragma once

#include "ddgl.h"
#include "ddglmath.h"

#include "ddclstorage.h"


#include <ft2build.h>
#include FT_FREETYPE_H
#include "freetype/freetype.h"
#include <freetype/ftglyph.h>

#define DDGL_TEXTURE_EXTEND 128

typedef struct tag_ddgl_Texture {
    int ref;
    ddcl_Handle h;
    ddgl_Size size;
    char extend[DDGL_TEXTURE_EXTEND];
    void(*free)(void * tex);
    void(*use)(void * tex);
}ddgl_Texture;

typedef struct tag_ddgl_FontFace{
    int ref;
    ddcl_Handle h;
    FT_Face face;
}ddgl_FontFace;

typedef struct tag_ddgl_TextDef {
    unsigned maxw;
    unsigned maxh;
    unsigned hgap;
    unsigned vgap;
    unsigned size;
}ddgl_TextDef;

DDGLAPI int
ddgl_init_texture_module(ddgl * conf);

DDGLAPI void
ddgl_exit_texture_module();

DDGLAPI ddgl_Texture *
ddgl_new_texture(ddgl_Size size);

DDGLAPI ddgl_Texture *
ddgl_retain_texture(ddcl_Handle h);

DDGLAPI void
ddgl_release_texture(ddcl_Handle h);

DDGLAPI ddgl_Texture *
ddgl_new_texture_with_png_file(const char * path);

DDGLAPI ddgl_Texture *
ddgl_new_texture_with_png_data(const char * data, size_t sz);

DDGLAPI ddgl_FontFace *
ddgl_new_fontface(const char * font);

DDGLAPI ddgl_FontFace *
ddgl_retain_fontface(ddcl_Handle h);

DDGLAPI void
ddgl_release_fontface(ddcl_Handle h);

DDGLAPI ddgl_Texture *
ddgl_new_texture_with_utf8(const char * str, ddgl_FontFace * font, ddgl_Texture * def);
