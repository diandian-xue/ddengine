#define DDGL_CORE

#include "ddglsprite.h"
#include "ddclstorage.h"

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

static const char DDGL_SPRITE_NAME[] = "ddgl_Sprite";
static ddcl_Storage * _S = NULL;

DDGLAPI void
ddgl_init_sprite_module(){
    if(_S){
        return;
    }
    _S = ddcl_new_storage(sizeof(ddgl_Sprite), 32);
}

DDGLAPI ddgl_Sprite *
ddgl_new_sprite(){
    ddgl_Sprite * sp;
    ddcl_Handle h = ddcl_register_in_storage(_S, &sp);
    sp->h = h;
    sp->visible = 1;
    sp->ref = 1;
    sp->position.w = 1;
    sp->scale.x = sp->scale.y = sp->scale.z = 1;
    sp->name = DDGL_SPRITE_NAME;
    ddgl_identity_matrix(&(sp->matrix));
    return sp;
}

DDGLAPI inline void
ddgl_retain_sprite(ddgl_Sprite * sp){
    sp->ref ++;
}

DDGLAPI void
ddgl_release_sprite(ddgl_Sprite * sp){
    sp->ref --;
    if(sp->ref > 0){
        return;
    }
    if(sp->free){
        sp->free(sp);
    }
    ddcl_del_in_storage(_S, sp->h);
}

DDGLAPI void
ddgl_add_child(ddgl_Sprite * sp, ddgl_Sprite * child){
    if(!child || child->parent){
        return;
    }
    child->parent = sp;
    child->next = NULL;
    if(sp->end){
        sp->end->next = child;
    }else{
        sp->begin = child;
    }
    child->last = sp->end;
    sp->end = child;
    ddgl_retain_sprite(child);
}

DDGLAPI void
ddgl_remvoe_child(ddgl_Sprite * sp, ddgl_Sprite * child){
    if(child->parent != sp){
        return;
    }
    if(child == sp->begin){
        sp->begin = child->next;
    }
    if(child == sp->end){
        sp->end = child->last;
    }
    if(child->last){
        child->last->next = child->next;
    }
    if(child->next){
        child->next->last = child->last;
    }
    child->parent = NULL;
    child->next = NULL;
    child->last = NULL;
    ddgl_release_sprite(child);
}

DDGLAPI inline void
ddgl_set_position(ddgl_Sprite * sp, ddgl_Vec * v){
    sp->position = *v;
    sp->dirty = 1;
}

DDGLAPI inline void
ddgl_set_x(ddgl_Sprite * sp, float x){
    sp->position.x = x;
    sp->dirty = 1;
}

DDGLAPI inline void
ddgl_set_y(ddgl_Sprite * sp, float y){
    sp->position.y = y;
    sp->dirty = 1;
}

DDGLAPI inline void
ddgl_set_z(ddgl_Sprite * sp, float z){
    sp->position.z = z;
    sp->dirty = 1;
}

static inline void
update_matrix(ddgl_Sprite * sp, ddgl_Matrix * pm){
    ddgl_Matrix matrix;
    ddgl_identity_matrix(&matrix);
    ddgl_translate_matrix_v(&matrix, &(sp->position));
    //matrix->V.v41 = sp->position.x;
    //matrix->V.v42 = sp->position.y;
    //matrix->V.v43 = sp->position.z;
    ddgl_scale_matrix_v(&matrix, &(sp->scale));
    if(sp->rotation.x != 0){
        ddgl_rotate_x_matrix(&matrix, sp->rotation.x);
    }
    if(sp->rotation.y != 0){
        ddgl_rotate_y_matrix(&matrix, sp->rotation.y);
    }
    if(sp->rotation.z != 0){
        ddgl_rotate_z_matrix(&matrix, sp->rotation.z);
    }
    sp->matrix = *pm;
    ddgl_product_matrix(&(sp->matrix), &matrix);
}

DDGLAPI void
ddgl_visit(ddgl_Sprite * sp, ddgl_Matrix * pm, int force){
    if(sp->visible && (force || sp->dirty)){
        update_matrix(sp, pm);
        force = 1;
    }
    if(sp->visit){
        sp->visit(sp, pm, force);
    }
    if(sp->begin){
        ddgl_Sprite * it = sp->begin;
        while(it){
            ddgl_visit(it, &(sp->matrix), force);
            it = it->next;
        }
    }
    sp->dirty = 0;
}

DDGLAPI int
ddgl_render(ddgl_Sprite * sp){
    if(!sp->visible)
        return 0;
    if(!sp->scale.x && !sp->scale.y && !sp->scale.z)
        return 0;
    if(sp->render){
        sp->render(sp);
    }
    if(sp->begin){
        ddgl_Sprite * iter = sp->begin;
        while(iter){
            ddgl_render(iter);
            iter = iter->next;
        }
    }
    return 1;

}
