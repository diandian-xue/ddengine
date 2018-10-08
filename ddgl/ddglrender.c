#define DDGL_CORE

#include "ddglrender.h"



#include <GL/glew.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFFER_OFFSET(C, B) &((C *)(void*)0)->B

static ddcl_Storage * _S;

DDGLAPI void
ddgl_init_render_module(ddgl * conf){
    _S = ddcl_new_storage(sizeof(ddgl_Render), 16);
}

DDGLAPI void
ddgl_exit_render_module(){
    ddcl_free_storage(_S);
}

DDGLAPI ddgl_Render *
ddgl_new_render(ddgl_Material * material){
    ddgl_Render * render;
    ddcl_Handle h = ddcl_register_in_storage(_S, &render);
    render->h = h;
    render->material = material;
    render->ref = 1;
	return render;
}

DDGLAPI ddgl_Render *
ddgl_retain_render(ddcl_Handle h){
    ddgl_Render * render = ddcl_find_in_storage(_S, h);
    if(render){
        render->ref ++;
    }
    return render;
}

DDGLAPI void
ddgl_release_render(ddgl_Render * render){
    render->ref --;
    if(render->ref <= 0){
        if(render->free){
            render->free(render);
        }
        ddcl_del_in_storage(_S, render->h);
    }
}

