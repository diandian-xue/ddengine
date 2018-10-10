#define DDGL_CORE

#include "ddglimage2d.h"


DDGLAPI ddcl_Handle DDGL_IMAGE2D_RENDER;
static const char * DDGL_IMAGE2D_NAME = "ddgl_Image2d";

typedef struct tag_Extend{
    ddgl_Texture * tex;
}Extend;

static void
free_image2d(ddgl_Image2d * sp){
    Extend * extend = (Extend *)sp->extend;
    ddgl_release_texture(extend->tex->h);
}

static inline ddgl_Vec
vec(float x, float y, ddgl_Matrix * m){
    ddgl_Vec v = { x, y, 0.0f, 1.0f };
    ddgl_product_vec_matrix(&v, m);
    return v;
}

static void
render_image2d(ddgl_Image2d * sp){
    float width = (sp->size.width * 0.5f) * sp->scale.x;
    float height = (sp->size.height * 0.5f) * sp->scale.y;
    width /= 500;
    height /= 500;
    if(sp->parent){
        ddgl_Sprite * parent = sp->parent;
        while(parent){
            width *= parent->scale.x;
            height *= parent->scale.y;
            parent = parent->parent;
        }
    }

    ddgl_Matrix * m = &(sp->matrix);
    ddgl_Vertex vertex[4] = {0};
    vertex[0].position = vec(-width, height, m);
    vertex[1].position = vec(-width, -height, m);
    vertex[2].position = vec(width, height, m);
    vertex[3].position = vec(width, -height, m);

    vertex[0].color = sp->color;
    vertex[1].color = sp->color;
    vertex[2].color = sp->color;
    vertex[3].color = sp->color;

    vertex[0].uv.u = 0;
    vertex[0].uv.v = 0;
    vertex[1].uv.u = 0;
    vertex[1].uv.v = 1;
    vertex[2].uv.u = 1;
    vertex[2].uv.v = 0;
    vertex[3].uv.u = 1;
    vertex[3].uv.v = 1;

    Extend * extend = (Extend *)sp->extend;
    extend->tex->use(extend->tex);
    ddgl_Render * render = ddgl_retain_render(DDGL_IMAGE2D_RENDER);
    render->draw(render, vertex, 4);
    ddgl_release_render(render);
}

DDGLAPI ddgl_Image2d *
ddgl_new_image2d(ddgl_Texture * tex){
    ddgl_Image2d * sp = ddgl_new_sprite();
    sp->name = DDGL_IMAGE2D_NAME;
    sp->free = free_image2d;
    sp->render = render_image2d;
    sp->size = tex->size;
    sp->color.r = 255;
    sp->color.g = 255;
    sp->color.b = 255;
    sp->color.a = 255;
    Extend * extend = (Extend *)sp->extend;
    extend->tex = tex;
    return sp;
}
