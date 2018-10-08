#define DDGL_CORE

#include "ddglimage2d.h"


DDGLAPI ddcl_Handle DDGL_IMAGE2D_RENDER;
static const char * DDGL_IMAGE2D_NAME = "ddgl_Image2d";

typedef struct tag_Extend{
    ddgl_Texture * tex;
}Extend;

static void
free_image2d(ddgl_Image2d * img){
    Extend * extend = (Extend *)img->extend;
    ddgl_release_texture(extend->tex->h);
}

static void
render_image2d(ddgl_Image2d * img){
}

DDGLAPI ddgl_Image2d *
ddgl_new_image2d(ddgl_Texture * tex){
    ddgl_Image2d * img = ddgl_new_sprite();
    img->name = DDGL_IMAGE2D_NAME;
    img->free = free_image2d;
    img->render = render_image2d;
    return img;
}
