#define DDGL_CORE

#include "ddgltexture.h"
#include "opengl/ddglopengl_texture.h"

#include "ddclfile.h"
#include "lodepng.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct tag_CharGlyph {
    FT_UInt index;
    unsigned ch;
    unsigned glyphw;
    unsigned glyphh;
    unsigned px;
    unsigned py;
}CharGlyph;

typedef struct tag_GlyphLine {
    unsigned pwidth;
    unsigned count;
    CharGlyph glyphs[100];
}GlyphLine;


static ddcl_Storage * _TexS;
static ddcl_Storage * _FontS;
static FT_Library _fontlib = NULL;

DDGLAPI int
ddgl_init_texture_module(ddgl * conf){
    _TexS = ddcl_new_storage(sizeof(ddgl_Texture), 32);
    _FontS = ddcl_new_storage(sizeof(ddgl_FontFace), 16);
    int err;
    err = FT_Init_FreeType(&_fontlib);
    if (err) {
        _fontlib = NULL;
        return 1;
    }
    return 0;
}

DDGLAPI void
ddgl_exit_texture_module(){
    ddcl_free_storage(_TexS);
    ddcl_free_storage(_FontS);
    FT_Done_FreeType(_fontlib);
}

DDGLAPI ddgl_Texture *
ddgl_new_texture(ddgl_Size size){
    ddgl_Texture * tex;
    ddcl_Handle h = ddcl_register_in_storage(_TexS, &tex);
    tex->h = h;
    tex->ref = 1;
    tex->size = size;

    switch(ddgl_conf()->engine){
    case DDGL_OPENGL:
        ddgl_gen_opengl_texture(tex);
        break;
    }
    return tex;
}

DDGLAPI ddgl_Texture *
ddgl_retain_texture(ddcl_Handle h){
    ddgl_Texture * tex = ddcl_find_in_storage(_TexS, h);
    if(tex){
        tex->ref ++;
    }
    return tex;
}

DDGLAPI void
ddgl_release_texture(ddcl_Handle h){
    ddgl_Texture * tex = ddcl_find_in_storage(_TexS, h);
    if(!tex){
        return;
    }
    tex->ref --;
    if(tex->ref <= 0){
        if(tex->free){
            tex->free(tex);
        }
        ddcl_del_in_storage(_TexS, h);
    }
}

DDGLAPI ddgl_Texture *
ddgl_new_texture_with_png_file(const char * path){
    char fbuff[DDCL_MAX_PATH];
    if (ddcl_full_path(path, fbuff)){
        return NULL;
    }
    unsigned error;
    unsigned char * png;
    unsigned width, height;
    error = lodepng_decode32_file(&png, &width, &height, path);
    if (error) {
        printf("lodepng_decode32_file error: %d %s \n", error, path);
        return NULL;
    }
    ddgl_Texture * tex;
    ddcl_Handle h = ddcl_register_in_storage(_TexS, &tex);
    tex->h = h;
    tex->ref = 1;
    tex->size.width = width;
    tex->size.height = height;
    ddgl_gen_opengl_texture_with_png(tex, png);
    free(png);
    return tex;
}

DDGLAPI ddgl_Texture *
ddgl_new_texture_with_png_data(const char * data, size_t sz){
    unsigned error;
    unsigned char * png;
    unsigned width, height;
    error = lodepng_decode32(&png, &width, &height, data, sz);
    if (error) {
        printf("lodepng_decode32 error: %d \n", error);
        return NULL;
    }
    ddgl_Texture * tex;
    ddcl_Handle h = ddcl_register_in_storage(_TexS, &tex);
    tex->h = h;
    tex->ref = 1;
    tex->size.width = width;
    tex->size.height = height;
    ddgl_gen_opengl_texture_with_png(tex, png);
    free(png);
    return tex;
}

DDGLAPI ddgl_FontFace *
ddgl_new_fontface(const char * font){
    unsigned err = 0;
    FT_Face face;
    err = FT_New_Face(_fontlib, font, 0, &face);
    if (err) {
        return NULL;
    }
    err = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
    if (err) {
        FT_Done_Face(face);
        return NULL;
    }

    ddgl_FontFace * ff;
    ddcl_Handle h = ddcl_register_in_storage(_FontS, &ff);
    ff->h = h;
    ff->ref = 1;
    ff->face = face;
    return ff;
}

DDGLAPI ddgl_FontFace *
ddgl_retain_fontface(ddcl_Handle h){
    ddgl_FontFace * ff = ddcl_find_in_storage(_FontS, h);
    if(ff){
        ff->ref ++;
    }
    return ff;
}

DDGLAPI void
ddgl_release_fontface(ddcl_Handle h){
    ddgl_FontFace * ff = ddcl_find_in_storage(_FontS, h);
    if(ff){
        ff->ref --;
        if(ff->ref <= 0){
            FT_Done_Face(ff->face);
            ddcl_del_in_storage(_FontS, h);
        }
    }
}

static unsigned
utf8(char **p)
{
    if ((**p & 0x80) == 0x00) {
        return  *((*p)++);
    }
    if ((**p & 0xE0) == 0xC0) {
        int a = *((*p)++) & 0x1F;
        int b = *((*p)++) & 0x3F;
        return (a << 6) | b;
    }
    if ((**p & 0xF0) == 0xE0) {
        int a = *((*p)++) & 0x0F;
        int b = *((*p)++) & 0x3F;
        int c = *((*p)++) & 0x3F;
        return (a << 12) | (b << 6) | c;
    }
    if ((**p & 0xF8) == 0xF0) {
        int a = *((*p)++) & 0x07;
        int b = *((*p)++) & 0x3F;
        int c = *((*p)++) & 0x3F;
        int d = *((*p)++) & 0x3F;
        return (a << 18) | (b << 12) | (c << 8) | d;
    }
    return 0;
}

static unsigned
divide_chars(const unsigned char * chars, FT_Face face, GlyphLine lines[100],
    ddgl_TextDef * def, unsigned * pw, unsigned * ph) {
    unsigned i = 0;
    unsigned l = 0;
    *pw = *ph = 0;
    unsigned glyphi = 0;
    FT_UInt index = 0;
    FT_UInt pre_index = 0;
    unsigned ch;
    FT_Vector delta;
    CharGlyph * cg;
    GlyphLine * gl;
    unsigned xoffet = 0;
    unsigned yoffet = 0;
    int hori_bearingx = 0;
    int hori_bearingy = 0;
    int hori_advance = 0;
    int kerning = 0;
    int ascender = face->size->metrics.ascender >> 6;
    int fh = face->size->metrics.height >> 6;
    unsigned max_advance = (face->size->metrics.max_advance >> 6);

    int hasKerning = FT_HAS_KERNING(face);

    const char* pchars = chars;
    while (ch = utf8((char**)&pchars)) {
        if (ch == 0)
            break;
        if (ch == ' ') {
            xoffet += max_advance * 0.5;
            ch = chars[++i];
            continue;
        }
        else if (ch == '\n' || ch=='\r') {
            if (ch == '\n') {
                xoffet = 0;
                glyphi = 0;
                l ++;
                yoffet += fh + def->vgap;
            }
            if (def->maxw && yoffet + fh > def->maxw)
                break;
            else {
                ch = chars[++i];
                continue;
            }
        }
        if (def->maxw && xoffet + (max_advance * 0.5f) > def->maxw) {
            ch = chars[++i];
            if (def->maxh && yoffet + fh + fh > def->maxh)
                break;
            else
                continue;
        }
        index = FT_Get_Char_Index(face, ch);
        if (FT_Load_Glyph(face, index, FT_LOAD_DEFAULT))
            return 0;
        gl = &(lines[l]);
        cg = &(gl->glyphs[glyphi++]);
        lines[l].count++;

        cg->index = index;
        cg->ch = ch;
        cg->glyphw = face->glyph->metrics.width >> 6;
        cg->glyphh = face->glyph->metrics.height >> 6;

        hori_bearingx = face->glyph->metrics.horiBearingX >> 6;
        hori_bearingy = face->glyph->metrics.horiBearingY >> 6;
        hori_advance = face->glyph->metrics.horiAdvance >> 6;

        cg->px = xoffet + hori_bearingx;
        xoffet += hori_advance + def->hgap;
        cg->py = yoffet + ascender - hori_bearingy;
        if (pre_index != 0 && hasKerning) {
            FT_Get_Kerning(face, pre_index, index, FT_KERNING_DEFAULT, &delta);
            kerning = delta.x >> 6;
            cg->px += kerning;
        }
        ch = chars[++i];
    }

    for (i = 0; i <= l; i++) {
        gl = &(lines[i]);
        if (gl->count) {
            unsigned w = gl->glyphs[gl->count - 1].px + max_advance;
            if (w > *pw)
                *pw = w;
        }
    }

    *ph = (l + 1) * fh + l * def->vgap;
    return l+1;
}

DDGLAPI ddgl_Texture *
ddgl_new_texture_with_utf8(const char * str, ddgl_FontFace * font, ddgl_TextDef * def){
    FT_Face face = font->face;
    FT_Set_Pixel_Sizes(face, def->size, def->size);
    GlyphLine lines[100] = { 0 };
    unsigned width, height;
    unsigned line_count = divide_chars(str, face, lines, def, &width, &height);

    CharGlyph * cg;
    GlyphLine * gl;
    FT_Error    err;
    FT_Glyph    glyph;
    unsigned char * big_map = calloc(1, sizeof(unsigned char) * 4 * width * height);

    for (unsigned i = 0; i < line_count; i++) {
        gl = &(lines[i]);
        for (unsigned j = 0; j < gl->count; j++) {
            cg = &(gl->glyphs[j]);
            err = FT_Load_Glyph(face, cg->index, FT_LOAD_DEFAULT);

            err = FT_Get_Glyph(face->glyph, &glyph);
            err = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
            FT_Bitmap * bitmap = &(((FT_BitmapGlyph)glyph)->bitmap);
            if (cg->ch == '2' && 0) {
                for (unsigned h = 0; h < bitmap->rows; h++) {
                    for (unsigned w = 0; w < bitmap->width; w++) {
                        printf("%d ", bitmap->buffer[h * bitmap->width + w]);
                    }
                    printf("\n");
                }
            }
            for (unsigned gh = 0; gh < cg->glyphh; gh++) {
                for (unsigned gw = 0; gw < cg->glyphw; gw++) {
                    if (bitmap->buffer[gh * cg->glyphw + gw]) {
                        big_map[((cg->py + gh) * height + (cg->px + gw)) * 4 + 3] = bitmap->buffer[gh * cg->glyphw + gw];
                    }
                }
            }
            FT_Done_Glyph(glyph);
        }
    }

    ddgl_Texture * tex;
    ddcl_Handle h = ddcl_register_in_storage(_TexS, &tex);
    tex->h = h;
    tex->ref = 1;
    tex->size.width = width;
    tex->size.height = height;
    ddgl_gen_opengl_texture_with_utf8map(tex, big_map);
    free(big_map);
    return tex;
}

