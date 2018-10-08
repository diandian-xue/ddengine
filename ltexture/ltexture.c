#define LTEXTURELUA_CORE

#include "ltexture.h"

#include "lodepng.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "freetype/freetype.h"
#include <freetype/ftglyph.h>

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

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

typedef struct tag_TextDef {
    unsigned maxw;
    unsigned maxh;
    unsigned hgap;
    unsigned vgap;
}TextDef;

FT_Library g_library = NULL;

static int
l_decode_png_file(lua_State * L){
    const char * file = luaL_checkstring(L, 1);
    unsigned error;
    unsigned char* image;
    unsigned width, height;
    error = lodepng_decode32_file(&image, &width, &height, file);
    if (error) {
        return luaL_error(L, "lodepng_decode32_file error: %d %s \n", error, file);
    }
    lua_pushlightuserdata(L, image);
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 3;
}

static int
l_decode_png_data(lua_State * L){
    void * data = NULL;
    size_t sz = 0;
    switch(lua_type(L, 1)){
    case LUA_TUSERDATA:
        data = lua_touserdata(L, 1);
        sz = luaL_checkinteger(L, 2);
        break;
    case LUA_TSTRING:
        data = lua_tolstring(L, 1, &sz);
        break;
    default:
        return luaL_error(L, "bad argument #1, string or userdata expected, got %s",
                lua_typename(L, lua_type(L, 1)));
    }

    unsigned error;
    unsigned char* image;
    unsigned width, height;

    error = lodepng_decode32(&image, &width, &height, data, sz);
    if(error){
        return luaL_error(L, "decode error %d", error);
    }
    lua_pushlightuserdata(L, image);
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 3;
}

static int
l_free(lua_State * L){
    void * data = lua_touserdata(L, 1);
    if(data){
        free(data);
    }
    return 0;
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
divide_chars(const unsigned char * chars, FT_Face face, GlyphLine * lines,
        TextDef * def, unsigned * pw, unsigned * ph) {
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
    FT_Error    err;
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
            if (def->maxh && yoffet + fh > def->maxh)
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
        int err = FT_Load_Glyph(face, index, FT_LOAD_DEFAULT);
        if (err)
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

static int
l_new_font_face(lua_State * L){
    const char * font = luaL_checkstring(L, 1);
    unsigned fontsize = luaL_checkinteger(L, 2);

    unsigned err = 0;
    if(!g_library){
        err = FT_Init_FreeType(&g_library);
        if (err) {
            g_library = NULL;
            return luaL_error(L, "FT_Init_FreeType error %d", err);
        }
    }

    FT_Face face;
    err = FT_New_Face(g_library, font, 0, &face);
    if (err) {
        return luaL_error(L, "FT_New_Face error %d", err);
    }

    //select utf8 charmap
    err = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
    if (err) {
        FT_Done_Face(face);
        return luaL_error(L, "FT_Select_Charmap  error %d", err);
    }
    FT_Set_Pixel_Sizes(face, fontsize, fontsize);
    lua_pushlightuserdata(L, face);
    return 1;
}

l_free_font_face(lua_State * L){
    FT_Face face = lua_touserdata(L, 1);
    if(face){
        FT_Done_Face(face);
    }
    return 0;
}

static int
l_data_with_string(lua_State * L) {
    luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
    FT_Face face = lua_touserdata(L, 1);
    const char * str = lua_tostring(L, 2);

    TextDef def = { 0 };
    if (lua_type(L, 3) == LUA_TTABLE) {
        char * ef_nv[4] = {
            "maxw",
            "maxh",
            "hgap",
            "vgap",
        };
        for (int i = 0; i < 4; i++) {
            lua_pushstring(L, ef_nv[i]);
            lua_gettable(L, 3);
            if (!lua_isnil(L, -1)) {
                *((unsigned *)&def + i) = lua_tointeger(L, -1);
            }
            lua_pop(L, 1);
        }
    }
    def.maxw = def.maxw == 0 || def.maxw > 1024 ? 1024 : 0;
    def.maxh = def.maxh == 0 || def.maxh > 1024 ? 1024 : 0;

    GlyphLine lines[100] = { 0 };
    unsigned pw, ph;
    unsigned line_count = divide_chars(str, face, lines, &def, &pw, &ph);

    CharGlyph * cg;
    GlyphLine * gl;
    FT_Error    err;
    FT_Glyph    glyph;
    unsigned char * big_map = calloc(1, sizeof(unsigned char) * 4 * pw * ph);
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
                        big_map[((cg->py + gh) * pw + (cg->px + gw)) * 4 + 3] = bitmap->buffer[gh * cg->glyphw + gw];
                    }
                }
            }
            FT_Done_Glyph(glyph);
        }
    }
    lua_pushlightuserdata(L, big_map);
    lua_pushinteger(L, pw);
    lua_pushinteger(L, ph);
    return 3;
}

static luaL_Reg _reg[] = {
    { "decode_png_file", l_decode_png_file },
    { "decode_png_data", l_decode_png_data },
    { "free", l_free },
    { "new_font_face", l_new_font_face },
    { "free_font_face", l_free_font_face },
    { "data_with_string", l_data_with_string },
    { NULL, NULL },
};


LTEXTURELUA int
luaopen_ltexture_core(lua_State * L){
    luaL_checkversion(L);
    DDLUA_NEWLIB(L, "ltexture.core", _reg);
    return 1;
}
