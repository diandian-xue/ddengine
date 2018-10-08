#define DDGL_CORE

#include "ddglshape2d.h"
#include "opengl/2d/ddglopengl_shape2d.h"

static const char * DDGL_SHAPE2D_NAME = "ddgl_Shape2d";
DDGLAPI ddcl_Handle DDGL_SHAPE2D_RENDER;

static void
free_shape2d(ddgl_Shape2d * sp){
}

static inline ddgl_Vec
vec(float x, float y, ddgl_Matrix * m){
    ddgl_Vec v = { x, y, 0.0f, 1.0f };
    ddgl_product_vec_matrix(&v, m);
    return v;
}

static void
render_shape2d(ddgl_Shape2d * sp){
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

    ddgl_Render * render = ddgl_retain_render(DDGL_SHAPE2D_RENDER);
    render->draw(render, vertex, 4);
    ddgl_release_render(render);
}

DDGLAPI ddgl_Shape2d *
ddgl_new_shape2d(){
    ddgl_Sprite * sp = ddgl_new_sprite();
    sp->name = DDGL_SHAPE2D_NAME;
    sp->free = free_shape2d;
    sp->render = render_shape2d;
    return sp;
}
