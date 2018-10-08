#define DDGL_CORE

#include "ddglwindow.h"
#include "ddglshader.h"
#include "ddglmaterial.h"
#include "ddglrender.h"

#include "opengl/ddglopengl.h"

#include <stdio.h>


#ifdef DDSYS_WIN
#include <Windows.h>
#endif


static ddcl_Storage * _S;

#define WINDOW_EXTEND  256


DDGLAPI int
ddgl_init_window_module (ddgl * conf){
    _S = ddcl_new_storage(sizeof(ddgl_Window), 8);
    return 0;
}

DDGLAPI void
ddgl_exit_window_module (){
    ddcl_free_storage(_S);
}

DDGLAPI ddgl_Window *
ddgl_new_window (const char * title, unsigned width, unsigned height){
    ddgl_Window * w;
    ddcl_Handle h = ddcl_register_in_storage(_S, &w);
    w->h = h;
    w->width = width;
    w->height = height;
    sprintf(w->title, "%s", title);

    ddgl * conf = ddgl_conf();
    switch(conf->engine){
    case DDGL_OPENGL:
        ddgl_new_opengl_window(w);
        break;
    }

    return w;
}

DDGLAPI int
ddgl_poll_window_event(){
#ifdef DDSYS_WIN
    MSG msg;
    if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) != 0){
        if (msg.message != WM_QUIT){
            DispatchMessage(&msg);
        }
    }
#endif

    return 0;
}

