#define DDGL_CORE

#include "opengl/ddglopengl.h"
#include "opengl/2d/ddglopengl_shape2d.h"
#include "opengl/2d/ddglopengl_image2d.h"


#ifdef DDSYS_WIN
    #include "opengl/platform/ddglopengl_win32.c"
#endif

DDGLAPI int
ddgl_init_opengl_module(ddgl * conf){
    ddgl_init_opengl_shape2d_module(conf);
    ddgl_init_opengl_image2d_module(conf);
    return 0;
}

DDGLAPI int
ddgl_exit_opengl_module(){

    return 0;
}
