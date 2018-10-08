#pragma once

#include "ddglconfig.h"
#include "ddglmath.h"
#include "ddclstorage.h"
#include "ddglrender.h"
#include "ddglmaterial.h"


struct tag_ddgl_Sprite;
typedef void (*ddgl_SpriteFree)(struct tag_ddgl_Sprite * sp);
typedef void (*ddgl_SpriteVisit)(struct tag_ddgl_Sprite * sp, ddgl_Matrix * pm, int force);
typedef void (*ddgl_SpriteRender)(struct tag_ddgl_Sprite * sp);
typedef ddgl_Vertex * (*ddgl_SpriteVertex)(struct tag_ddgl_Sprite * sp, int * count);

#define DDGL_SPRITE_EXTEND  32

struct tag_ddgl_Sprite{
    int ref;
    ddcl_Handle h;
    ddgl_Vec position;
    ddgl_Vec scale;
    ddgl_Color color;
    ddgl_Vec rotation;
	ddgl_Size size;
    int visible;
    int dirty;
    const char * name;

    struct tag_ddgl_Sprite * parent;
    struct tag_ddgl_Sprite * last;
    struct tag_ddgl_Sprite * next;

    struct tag_ddgl_Sprite * begin;
    struct tag_ddgl_Sprite * end;

    ddgl_Matrix matrix;

    void * ud;
    char extend[DDGL_SPRITE_EXTEND];
    ddgl_SpriteFree free;
    ddgl_SpriteVisit visit;
    ddgl_SpriteRender render;
    ddgl_SpriteVertex vertex;
    ddgl_Material * material;
};

typedef struct tag_ddgl_Sprite ddgl_Sprite;

DDGLAPI void
ddgl_init_sprite_module();

DDGLAPI ddgl_Sprite *
ddgl_new_sprite();

DDGLAPI inline void
ddgl_retain_sprite(ddgl_Sprite * sp);

DDGLAPI void
ddgl_release_sprite(ddgl_Sprite * sp);

DDGLAPI void
ddgl_add_child(ddgl_Sprite * sp, ddgl_Sprite * child);

DDGLAPI void
ddgl_remvoe_child(ddgl_Sprite * sp, ddgl_Sprite * child);

DDGLAPI inline void
ddgl_set_position(ddgl_Sprite * sp, ddgl_Vec * v);

DDGLAPI inline void
ddgl_set_x(ddgl_Sprite * sp, float x);

DDGLAPI inline void
ddgl_set_y(ddgl_Sprite * sp, float y);

DDGLAPI inline void
ddgl_set_z(ddgl_Sprite * sp, float z);

DDGLAPI void
ddgl_visit(ddgl_Sprite * sp, ddgl_Matrix * m, int force);

DDGLAPI int
ddgl_render(ddgl_Sprite * sp);
