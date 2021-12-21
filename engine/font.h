
#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "pch.hpp"

struct BMPImage {
    std::shared_ptr<int[]> pixels;
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

    // how tall the tallest letter is
    float ascent;
    // how deep the deepest letter is
    float descent;
    // recommended vert distance from decent to acent of next line
    float linegap;
    // ??? actual vertical distance?
    float lineadvance;

    int size;
};
static std::map<std::string, std::shared_ptr<Font> > fonts;

// takes 2 codepoints and finds the kerning needed for it
inline float getKerning(const std::shared_ptr<Font> font, int first_cp,
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

void load_font_file(const char* filename) {
    log_info("loading font {}", filename);

    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size == -1) {
        log_error("Failed to load font {}, file not found", filename);
        return;
    }

    // TODO are the font files bigger than this?
    char buffer[1 << 25];
    if (file.read(buffer, size)) {
        // stb_tt wants unsigned char :shrug:
        unsigned char* ttf_buffer = (unsigned char*)buffer;

        // make a new font
        std::shared_ptr<Font> font = std::make_shared<Font>();
        font->name = nameFromFilePath(filename);
        font->size = 100;

        stbtt_fontinfo info;
        stbtt_InitFont(&info, ttf_buffer, 0);
        float scale = stbtt_ScaleForPixelHeight(&info, font->size);
        int s_ascent, s_decent, s_linegap;
        stbtt_GetFontVMetrics(&info, &s_ascent, &s_decent, &s_linegap);

        font->ascent = 1.f * s_ascent * scale;
        font->descent = 1.f * s_decent * scale;
        font->linegap = 1.f * s_linegap * scale;
        font->lineadvance = font->ascent - font->descent + font->linegap;

        unsigned char* bitmap;

        for (int codepoint = ' '; codepoint <= '~'; codepoint++) {
            int s_advance;
            int s_leftbearing;
            stbtt_GetCodepointHMetrics(&info, codepoint, &s_advance,
                                       &s_leftbearing);
            float scale = stbtt_ScaleForPixelHeight(&info, font->size);

            int s_w, s_h;
            int s_xoff, s_yoff;
            bitmap = stbtt_GetCodepointBitmap(&info, 0, scale, codepoint, &s_w,
                                              &s_h, &s_xoff, &s_yoff);

            int border = 3;
            int glyph_width = s_w + 2 * border;
            int glyph_height = s_h + 2 * border;
            int img_size = glyph_width * glyph_height;

            std::shared_ptr<int[]> image(new int[img_size]);

            int j, i;
            for (j = 0; j < s_h; ++j) {
                for (i = 0; i < s_w; ++i) {
                    int y = s_h - j;
                    int px = (j + border) * glyph_width + (i + border);
                    float val = bitmap[y * s_w + i] / 255.f;
                    glm::vec4 col = glm::vec4{val};
                    int packed = packRGBA(col);
                    image[px] = packed;
                }
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

            std::string texname =
                fmt::format("{}_{}", font->name, std::to_string(codepoint));
            std::shared_ptr<Texture> letterTexture =
                std::make_shared<Texture2D>(texname, glyph_width, glyph_height);
            letterTexture->setBitmapData((void*)image.get());
            letterTexture->tilingFactor = 1.f;
            textureLibrary.add(letterTexture);

            Glyph g;
            g.textureName = texname;
            g.codepoint = codepoint;
            g.advance = 1.f * s_advance * scale;
            g.leftbearing = 1.f * s_leftbearing * scale;
            g.xoff = s_xoff - border;
            g.yoff = s_yoff - border;
            g.image = BMPImage({
                .width = glyph_width,                         //
                .height = glyph_height,                       //
                .pixels = image,                              //
                .aspect = (1.f * glyph_width) / glyph_height  //
            });
            font->glyphs.push_back(g);
            stbtt_FreeBitmap(bitmap, 0);
        }

        fonts[font->name] = font;
    }
}

