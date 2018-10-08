#define DDGL_CORE

#include "ddgl.h"

#include "ddglwindow.h"
#include "ddglsprite.h"
#include "ddglshader.h"
#include "ddglmaterial.h"
#include "ddglrender.h"
#include "ddgltexture.h"

#include "opengl/ddglopengl.h"

static ddgl _conf;

DDGLAPI int
ddgl_init(ddgl * conf){
    _conf = *conf;
    ddgl_init_sprite_module();
    ddgl_init_shader_module(conf);
    ddgl_init_material_module(conf);
    ddgl_init_render_module(conf);
    ddgl_init_window_module(conf);
    ddgl_init_texture_module(conf);
    return 0;
}

DDGLAPI void
ddgl_default(ddgl * conf){
    conf->engine = DDGL_OPENGL;
}

DDGLAPI void
ddgl_init_modules(){
}

DDGLAPI ddgl *
ddgl_conf(){
    return &_conf;
}
