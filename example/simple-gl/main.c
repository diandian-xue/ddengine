
#include "ddcl.h"
#include "ddclservice.h"
#include "ddclstorage.h"

#include "ddgl.h"
#include "ddglwindow.h"
#include "ddglsprite.h"
#include "opengl/2d/ddglopengl_shape2d.h"
#include "2d/ddglshape2d.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

static ddcl_Service _mainsvr;
static ddgl_Window * _window;
static ddgl_Shape2d * _shape2d;
static ddgl_Shape2d * _child;

static void
poll_window_event(){
    ddcl_timeout(_mainsvr, NULL, 17);
    ddgl_poll_window_event(_window);
}

static void
mainsvr_msgcb(ddcl_Msg * msg){
    switch(msg->cmd){
    case DDCL_CMD_TIMEOUT:
        poll_window_event();
        _window->make_current(_window);
        _window->clear(_window);

        ddgl_set_x(_shape2d, _shape2d->position.x + 0.002);
        _shape2d->rotation.z += 0.01;
        _shape2d->dirty = 1;
        ddgl_set_x(_child, 0.1);
        _child->rotation.z += 0.01;
        _child->dirty = 1;

        ddgl_Matrix m;
        ddgl_identity_matrix(&m);
        ddgl_visit(_shape2d, &m, 0);

        ddgl_render(_shape2d);
        _window->swap_buffer(_window);
        _window->make_current(NULL);
        break;
    }
}


int main(){
    ddcl conf;
    ddcl_default(&conf);
    int ret = ddcl_init(&conf);
    if(ret){
        printf("ddcl_init failed %d\n", ret);
        return 1;
    }

    ddgl glconf;
    ddgl_default(&glconf);
    ddgl_init(&glconf);
    _window = ddgl_new_window("simple-gl", 500, 500);

    _shape2d = ddgl_new_shape2d();
    _shape2d->size.width = 200;
    _shape2d->size.height = 200;
    _shape2d->color.a = 255;
    _shape2d->color.r = 255;

    _child = ddgl_new_shape2d();
    _child->position.y = 0.5;
    _child->size.width = 100;
    _child->size.height = 100;
    _child->color.a = 255;
    _child->color.b = 255;
    ddgl_add_child(_shape2d, _child);

    for(int i = 0; i < 0; i++){
        ddgl_Shape2d * child = ddgl_new_shape2d();
        child->position.y = -0.5;
        child->size.width = 100;
        child->size.height = 100;
        child->color.a = 255;
        child->color.g = 255;
        ddgl_add_child(_shape2d, child);
    }


    _mainsvr = ddcl_new_service_not_worker(mainsvr_msgcb, NULL);

    poll_window_event();
    ddcl_start(_mainsvr);
    return 0;
}
