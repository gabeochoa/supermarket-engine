
#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "external_include.h"
#include "strutil.h"
#include "texture.h"

struct BMPImage {
    unsigned char* pixels;
    int width;
    int height;
    float aspect;
};

struct Glyph {
    std::string textureName;
    // TODO replace with texture
    BMPImage image;

    int codepoint;
    float advance;
    float leftbearing;
    float xoff;
    float yoff;
    glm::vec2 minUV;
    glm::vec2 maxUV;
};

struct Font {
    std::string name;
    std::vector<Glyph> glyphs;
    std::vector<float> kerning;

    float ascent;
    float descent;
    float linegap;

    float lineadvance;

    int size;
};
static std::map<std::string, std::shared_ptr<Font> > fonts;

// takes 2 codepoints and finds the kerning needed for it
static float getKerning(const std::shared_ptr<Font> font, int first_cp,
                        int second_cp) {
    int glyphIndex = first_cp - ' ';
    int nextIndex = -1;
    if (second_cp != 0) {
        nextIndex = second_cp - ' ';
    }
    float kerning = 0.f;
    if (nextIndex != -1) {
        kerning = font->kerning[glyphIndex * font->glyphs.size() + nextIndex];
    }
    return kerning;
}

inline uint32_t packRGBA(glm::vec4 color) {
    return (uint32_t)((color.r * 255.0f + 0.5f)) |
           ((uint32_t)((color.g * 255.0f) + 0.5f) << 8) |
           ((uint32_t)((color.b * 255.0f) + 0.5f) << 16) |
           ((uint32_t)((color.a * 255.0f) + 0.5f) << 24);
}

static void load_font_file(const char* filename) {
    log_info("loading font {}", filename);

    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size == -1) {
        log_error("Failed to load font {}, file not found", filename);
        return;
    }

    char* buffer = new char[size];

    if (file.read(buffer, size)) {
        std::shared_ptr<Font> font = std::make_shared<Font>();

        font->name = nameFromFilePath(filename);
        font->size = 30;

        stbtt_fontinfo info;
        stbtt_InitFont(&info, (unsigned char*)buffer, 0);

        float scale = stbtt_ScaleForPixelHeight(&info, font->size);

        int s_ascent;
        int s_decent;
        int s_linegap;
        stbtt_GetFontVMetrics(&info, &s_ascent, &s_decent, &s_linegap);

        font->ascent = 1.f * s_ascent * scale;
        font->descent = 1.f * s_decent * scale;
        font->linegap = 1.f * s_linegap * scale;

        font->lineadvance = font->ascent - font->descent + font->linegap;

        for (int i = ' '; i <= '~'; i++) {
            Glyph g;
            int s_advance;
            int s_leftbearing;
            stbtt_GetCodepointHMetrics(&info, i, &s_advance, &s_leftbearing);

            g.codepoint = i;
            g.advance = 1.f * s_advance * scale;
            g.leftbearing = 1.f * s_leftbearing * scale;

            int s_w;
            int s_h;
            int s_xoff;
            int s_yoff;

            unsigned char* bitmap = stbtt_GetCodepointBitmap(
                &info, 0, scale, i, &s_w, &s_h, &s_xoff, &s_yoff);

            if (s_w > 20000) s_w = 0;
            if (s_h > 20000) s_h = 0;

            if (s_w == 0) {
                continue;
            }

            // std::shared_ptr<Texture> tex = std::make_shared<Texture2D>(
            // fmt::format("{}_{}", font->name, i), s_w, s_h);
            // tex->setBitmapData(&bitmap);
            // textureLibrary.add(tex);

            int border = 3;
            int g_w = s_w + 2 * border;
            int g_h = s_h + 2 * border;
            int img_size = g_w * g_h * 4;

            unsigned char* texmem = (unsigned char*)calloc(img_size, 1);

            g.image = BMPImage({
                .width = g_w,
                .height = g_h,
                .pixels = texmem,
                .aspect = (1.f * g_w) / g_h,
            });

            g.xoff = s_xoff - border;
            g.yoff = s_yoff - border;

            for (int y = 0; y < s_h; y++) {
                for (int x = 0; x < s_w; x++) {
                    int pxindex = y * s_w + x;
                    char tone = bitmap[pxindex];
                    float tone01 = tone / 255.f;
                    glm::vec4 color(tone01);
                    int packedColor = packRGBA(color);
                    int targetPx = (y + border) * g_w + (x + border);
                    int* im_pixels = (int*)texmem;
                    im_pixels[targetPx] = packedColor;
                }
            }
            stbtt_FreeBitmap(bitmap, 0);
            font->glyphs.push_back(g);
        }

        for (size_t i = 0; i < font->glyphs.size(); i++) {
            for (size_t j = 0; j < font->glyphs.size(); j++) {
                // TODO actually put kern in the list,
                // not doing it now since need to make the index correctly
                //
                // int idx = i * font->glyphs.size() + j;
                // int a = font->glyphs[i].codepoint;
                // int b = font->glyphs[j].codepoint;
                // int kern = stbtt_GetCodepointKernAdvance(&info, a, b);
                font->kerning.push_back(10.f);
            }
        }

        fonts[font->name] = font;
    }
    delete[] buffer;
}

