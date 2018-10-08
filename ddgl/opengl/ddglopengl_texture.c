#define DDGL_CORE


#include "opengl/ddglopengl_texture.h"

#include <GL/glew.h>

typedef struct tag_TextureExtend{
    GLuint id;
}TextureExtend;

static void
use_texture(ddgl_Texture * tex){
    TextureExtend * extend = (TextureExtend *)tex->extend;
    glBindTexture(GL_TEXTURE_2D, extend->id);
}

static void
free_texture(ddgl_Texture * tex){
    TextureExtend * extend = (TextureExtend *)tex->extend;
    glDeleteTextures(1, &(extend->id));
}

DDGLAPI int
ddgl_gen_opengl_texture(ddgl_Texture * tex){
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    TextureExtend * extend = (TextureExtend *)tex->extend;
    extend->id = id;
    tex->use = use_texture;
    tex->free = free_texture;

    ddgl_Size sz = tex->size;

    GLenum sw[4] = { GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA };
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, sw);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, sz.width, sz.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    return 0;
}

DDGLAPI int
ddgl_gen_opengl_texture_with_png(ddgl_Texture * tex, const char * png){
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    TextureExtend * extend = (TextureExtend *)tex->extend;
    extend->id = id;
    tex->use = use_texture;
    tex->free = free_texture;

    ddgl_Size sz = tex->size;

    GLenum sw[4] = { GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA };
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, sw);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, sz.width, sz.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, png);

    return 0;
}

DDGLAPI int
ddgl_gen_opengl_texture_with_utf8map(ddgl_Texture * tex, const char * map){
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    TextureExtend * extend = (TextureExtend *)tex->extend;
    extend->id = id;
    tex->use = use_texture;
    tex->free = free_texture;

    ddgl_Size sz = tex->size;

    GLenum sw[4] = { GL_ONE, GL_ONE, GL_ONE, GL_ALPHA };
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, sw);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, sz.width, sz.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, map);
    return 0;
}
