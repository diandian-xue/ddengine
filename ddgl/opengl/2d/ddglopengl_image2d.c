#define DDGL_CORE

#include "2d/ddglimage2d.h"
#include "opengl/2d/ddglopengl_image2d.h"

#include "opengl/ddglopengl_shader.h"
#include "ddglshader.h"
#include "ddglmaterial.h"
#include "ddglrender.h"
#include "ddglmath.h"

#include <GL/glew.h>

ddcl_Handle DDGL_OPENGL_IMAGE2D_SHADER;
ddcl_Handle DDGL_OPENGL_IMAGE2D_MATERIAL;
ddcl_Handle DDGL_OPENGL_IMAGE2D_RENDER;

const char DDGL_OPENGL_IMAGE2D_SHADER_VERTS[] = " \n\
    out vec4 v_color; \n\
    out vec2 v_uv; \n\
    void main(){ \n\
        gl_Position = position; \n\
        v_color = color; \n\
        v_uv = uv; \n\
    }\n";
const char DDGL_OPENGL_IMAGE2D_SHADER_FRAGS[] = " \n\
    in vec4 v_color; \n\
    in vec2 v_uv; \n\
    uniform sampler2D texture; \n\
    void main(){ \n\
        gl_FragColor = texture2D(texture, v_uv) * v_color; \n\
    }\n";

typedef struct tag_RenderExtend{
    GLuint VAO;
    GLuint ABO;
    GLuint EBO;
}RenderExtend;

static void
free_render(ddgl_Render * render){
    RenderExtend * extend = (RenderExtend*)render->extend;
}

static void
draw_render(ddgl_Render * render, ddgl_Vertex * vertex, int count){
    RenderExtend * extend = (RenderExtend*)render->extend;
    glBindVertexArray(extend->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, extend->ABO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ddgl_Vertex) * count, vertex,  GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, extend->EBO);
    ddgl_Shader * shader = render->material->shader;
    shader->use(shader);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
}

DDGLAPI int
ddgl_init_opengl_image2d_module(ddgl * conf){
    ddgl_Shader * shader = ddgl_new_shader();
    DDGL_OPENGL_IMAGE2D_SHADER = shader->h;

    ddgl_GenOpenglShader gen;
    gen.verts[0] = DDGL_OPENGL_IMAGE2D_SHADER_VERTS;
    gen.vcount = 1;
    gen.frags[0] = DDGL_OPENGL_IMAGE2D_SHADER_FRAGS;
    gen.fcount = 1;
    ddgl_gen_opengl_shader(shader, &gen);

    ddgl_Material * material = ddgl_new_material();
    DDGL_OPENGL_IMAGE2D_MATERIAL = material->h;
    material->shader = shader;

    ddgl_Render * render = ddgl_new_render(material);
    DDGL_OPENGL_IMAGE2D_RENDER = render->h;
    DDGL_IMAGE2D_RENDER = DDGL_OPENGL_IMAGE2D_RENDER;
    RenderExtend * extend = (RenderExtend*)render->extend;
    glGenVertexArrays(1, &(extend->VAO));
    glBindVertexArray(extend->VAO);
    glGenBuffers(1, &(extend->ABO));
    glBindBuffer(GL_ARRAY_BUFFER, extend->ABO);
    ddgl_set_opengl_vbo_vertex_attrib();

    glGenBuffers(1, &(extend->EBO));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, extend->EBO);
    GLushort indexs[6] = { 0, 1, 2, 3, 2, 1};
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexs), indexs, GL_STATIC_DRAW);
    render->free = free_render;
    render->draw = draw_render;
    return 0;
}

DDGLAPI void
ddgl_exit_opengl_image2d_module(){
    ddgl_release_shader(DDGL_OPENGL_IMAGE2D_SHADER);
    ddgl_release_render(DDGL_OPENGL_IMAGE2D_RENDER);
}
