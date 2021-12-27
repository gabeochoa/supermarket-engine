
#pragma once

//
#include <stdio.h>
#include <stdlib.h>

#include "pch.hpp"
//
#include "../engine/strutil.h"

const int START_CODEPOINT = 32;
const int END_CODEPOINT = 30922;
const int FONT_SIZE = 64 * 2;
const int ITEMS_PER_ROW = 50;
const int NUM_LETTERS = END_CODEPOINT - START_CODEPOINT;

struct BMPImage {
    float aspect;
    int height;
    std::shared_ptr<int[]> pixels;
    int width;
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
    std::shared_ptr<Texture> texture;

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

inline uint32_t packRGBA(glm::vec4 color) {
    return (uint32_t)((color.r * 255.0f + 0.5f)) |
           ((uint32_t)((color.g * 255.0f) + 0.5f) << 8) |
           ((uint32_t)((color.b * 255.0f) + 0.5f) << 16) |
           ((uint32_t)((color.a * 255.0f) + 0.5f) << 24);
}

std::shared_ptr<Texture> fetch_texture_for_sentence(
    const char* fontname, const std::basic_string<char> phrase,
    bool temporary = false) {
    std::string filename = fmt::format("./resources/fonts/{}.ttf", fontname);

    // This "_" is okay since we are never splitting the string
    // into pieces, so its okay if phrase has underscore
    // if we ever decide to split then be careful
    auto textureName = fmt::format("{}_{}", fontname, phrase);

    if (textureLibrary.hasMatchingTexture(textureName)) {
        auto ptr = textureLibrary.get(textureName);
        if (ptr) {
            return ptr;
        }
    }

    // otherwise we have to generate it

    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    if (size == -1) {
        log_error("Failed to load font {}, file not found", filename);
        return nullptr;
    }

    // TODO are the font files bigger than this?
    char* buffer = (char*)calloc(size, sizeof(unsigned char));

    if (file.read(buffer, size)) {
        auto fontBuffer = (unsigned char*)buffer;

        /* Initialize font */
        stbtt_fontinfo info;
        if (!stbtt_InitFont(&info, fontBuffer, 0)) {
            log_error("Failed to load font {}", filename);
        }

        /* Calculate font scaling */
        float pixels = 1.5f * FONT_SIZE; /* Font size (font size) */
        float scale = stbtt_ScaleForPixelHeight(
            &info, pixels); /* scale = pixels / (ascent - descent) */

        /* create a bitmap */
        int bitmap_w = pixels * phrase.size(); /* Width of bitmap */
        int bitmap_h = pixels;                 /* Height of bitmap */
        unsigned char* bitmap =
            (unsigned char*)calloc(bitmap_w * bitmap_h, sizeof(unsigned char));

        /**
         * Get the measurement in the vertical direction
         * ascent: The height of the font from the baseline to the top;
         * descent: The height from baseline to bottom is usually negative;
         * lineGap: The distance between two fonts;
         * The line spacing is: ascent - descent + lineGap.
         */
        int ascent = 0;
        int descent = 0;
        int lineGap = 0;
        stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

        /* Adjust word height according to zoom */
        ascent = roundf(ascent * scale);
        descent = roundf(descent * scale);

        int x = 0; /*x of bitmap*/

        /* Cyclic loading of each character in word */
        for (size_t i = 0; i < phrase.size(); ++i) {
            /**
             * Get the measurement in the horizontal direction
             * advanceWidth: Word width;
             * leftSideBearing: Left side position;
             */
            int advanceWidth = 0;
            int leftSideBearing = 0;
            stbtt_GetCodepointHMetrics(&info, phrase[i], &advanceWidth,
                                       &leftSideBearing);

            /* Gets the border of a character */
            int c_x1, c_y1, c_x2, c_y2;
            stbtt_GetCodepointBitmapBox(&info, phrase[i], scale, scale, &c_x1,
                                        &c_y1, &c_x2, &c_y2);

            /* Calculate the y of the bitmap (different characters have
             * different heights) */
            int y = ascent + c_y1;

            /* Render character */
            int byteOffset =
                x + roundf(leftSideBearing * scale) + (y * bitmap_w);
            stbtt_MakeCodepointBitmap(&info, bitmap + byteOffset, c_x2 - c_x1,
                                      c_y2 - c_y1, bitmap_w, scale, scale,
                                      phrase[i]);

            /* Adjust x */
            x += roundf(advanceWidth * scale);

            /* kerning */
            int kern;
            kern =
                stbtt_GetCodepointKernAdvance(&info, phrase[i], phrase[i + 1]);
            x += roundf(kern * scale);
        }

        /* Save the bitmap data to the 1-channel png image */
        // stbi_write_png("STB.png", bitmap_w, bitmap_h, 1, bitmap, bitmap_w);

        for (int i = 0; i < bitmap_h / 2; i++) {
            int k = bitmap_h - 1 - i;
            for (int j = 0; j < bitmap_w; j++) {
                unsigned char tmp = bitmap[i * bitmap_w + j];
                bitmap[i * bitmap_w + j] = bitmap[k * bitmap_w + j];
                bitmap[k * bitmap_w + j] = tmp;
            }
        }

        std::shared_ptr<Texture> fontTexture =
            std::make_shared<Texture2D>(textureName, bitmap_w, bitmap_h);
        fontTexture->setBitmapData(bitmap);
        fontTexture->tilingFactor = 1.f;
        fontTexture->temporary = temporary;
        textureLibrary.add(fontTexture);

        free(fontBuffer);
        free(bitmap);
        return fontTexture;
    }
    return nullptr;
}

std::shared_ptr<Texture> fetch_texture_for_intl_phrase(
    const char* fontname, const std::wstring& phrase, bool temporary = false) {
    std::string filename = fmt::format("./resources/fonts/{}.ttf", fontname);

    // This "_" is okay since we are never splitting the string
    // into pieces, so its okay if phrase has underscore
    // if we ever decide to split then be careful
    auto textureName = fmt::format("{}_{}", fontname, to_string(phrase));

    if (textureLibrary.hasMatchingTexture(textureName)) {
        auto ptr = textureLibrary.get(textureName);
        if (ptr) {
            return ptr;
        }
    }

    // otherwise we have to generate it

    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    if (size == -1) {
        log_error("Failed to load font {}, file not found", filename);
        return nullptr;
    }

    // TODO are the font files bigger than this?
    char* buffer = (char*)calloc(size, sizeof(unsigned char));

    if (file.read(buffer, size)) {
        auto fontBuffer = (unsigned char*)buffer;

        /* Initialize font */
        stbtt_fontinfo info;
        if (!stbtt_InitFont(&info, fontBuffer, 0)) {
            log_error("Failed to load font {}", filename);
        }

        /* Calculate font scaling */
        float pixels = 1.5f * FONT_SIZE; /* Font size (font size) */
        float scale = stbtt_ScaleForPixelHeight(
            &info, pixels); /* scale = pixels / (ascent - descent) */

        /* create a bitmap */
        int bitmap_w = pixels * phrase.size(); /* Width of bitmap */
        int bitmap_h = pixels;                 /* Height of bitmap */
        unsigned char* bitmap =
            (unsigned char*)calloc(bitmap_w * bitmap_h, sizeof(unsigned char));

        /**
         * Get the measurement in the vertical direction
         * ascent: The height of the font from the baseline to the top;
         * descent: The height from baseline to bottom is usually negative;
         * lineGap: The distance between two fonts;
         * The line spacing is: ascent - descent + lineGap.
         */
        int ascent = 0;
        int descent = 0;
        int lineGap = 0;
        stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

        /* Adjust word height according to zoom */
        ascent = roundf(ascent * scale);
        descent = roundf(descent * scale);

        int x = 0; /*x of bitmap*/

        /* Cyclic loading of each character in word */
        for (size_t i = 0; i < phrase.size(); ++i) {
            /**
             * Get the measurement in the horizontal direction
             * advanceWidth: Word width;
             * leftSideBearing: Left side position;
             */
            int advanceWidth = 0;
            int leftSideBearing = 0;
            stbtt_GetCodepointHMetrics(&info, phrase[i], &advanceWidth,
                                       &leftSideBearing);

            /* Gets the border of a character */
            int c_x1, c_y1, c_x2, c_y2;
            stbtt_GetCodepointBitmapBox(&info, phrase[i], scale, scale, &c_x1,
                                        &c_y1, &c_x2, &c_y2);

            /* Calculate the y of the bitmap (different characters have
             * different heights) */
            int y = ascent + c_y1;

            /* Render character */
            int byteOffset =
                x + roundf(leftSideBearing * scale) + (y * bitmap_w);
            stbtt_MakeCodepointBitmap(&info, bitmap + byteOffset, c_x2 - c_x1,
                                      c_y2 - c_y1, bitmap_w, scale, scale,
                                      phrase[i]);

            /* Adjust x */
            x += roundf(advanceWidth * scale);

            /* kerning */
            int kern;
            kern =
                stbtt_GetCodepointKernAdvance(&info, phrase[i], phrase[i + 1]);
            x += roundf(kern * scale);
        }

        /* Save the bitmap data to the 1-channel png image */
        // stbi_write_png("STB.png", bitmap_w, bitmap_h, 1, bitmap, bitmap_w);

        for (int i = 0; i < bitmap_h / 2; i++) {
            int k = bitmap_h - 1 - i;
            for (int j = 0; j < bitmap_w; j++) {
                unsigned char tmp = bitmap[i * bitmap_w + j];
                bitmap[i * bitmap_w + j] = bitmap[k * bitmap_w + j];
                bitmap[k * bitmap_w + j] = tmp;
            }
        }

        std::shared_ptr<Texture> fontTexture =
            std::make_shared<Texture2D>(textureName, bitmap_w, bitmap_h);
        fontTexture->setBitmapData(bitmap);
        fontTexture->tilingFactor = 1.f;
        fontTexture->temporary = temporary;
        textureLibrary.add(fontTexture);

        free(fontBuffer);
        free(bitmap);
        return fontTexture;
    }
    return nullptr;
}

void load_font_textures(const char* filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    if (size == -1) {
        log_error("Failed to load font {}, file not found", filename);
        return;
    }

    std::string fontname = nameFromFilePath(filename);

    // TODO is there a way we can just load these from the font
    auto num_letters = NUM_LETTERS;
    auto end_codepoint = END_CODEPOINT;
    if (num_letters > 200) {
        end_codepoint = START_CODEPOINT + 200;
        num_letters = 200;
        log_warn(
            "due to issues with truetype segfaulting on giant bitmaps, "
            "limiting your codepoints to just the first 200");
    }

    // TODO are the font files bigger than this?
    char* buffer = (char*)calloc(size, sizeof(unsigned char));
    if (file.read(buffer, size)) {
        auto fontBuffer = (unsigned char*)buffer;

        /* Initialize font */
        stbtt_fontinfo info;
        if (!stbtt_InitFont(&info, fontBuffer, 0)) {
            log_error("Failed to load font {}, file not found", filename);
        }

        /* Calculate font scaling */
        float pixels = 1.f * FONT_SIZE; /* Font size (font size) */
        float scale = stbtt_ScaleForPixelHeight(
            &info, pixels); /* scale = pixels / (ascent - descent) */

        /* create a bitmap */
        int bitmap_w = pixels * ITEMS_PER_ROW; /* Width of bitmap */
        int bitmap_h =
            pixels * (num_letters / ITEMS_PER_ROW); /* Height of bitmap */

        log_warn("w{} h{} size {}", bitmap_w, bitmap_h,
                 bitmap_w * bitmap_h * sizeof(unsigned char));

        unsigned char* bitmap =
            (unsigned char*)calloc(bitmap_w * bitmap_h, sizeof(unsigned char));

        /**
         * Get the measurement in the vertical direction
         * ascent: The height of the font from the baseline to the top;
         * descent: The height from baseline to bottom is usually negative;
         * lineGap: The distance between two fonts;
         * The line spacing is: ascent - descent + lineGap.
         */
        int ascent = 0;
        int descent = 0;
        int lineGap = 0;
        stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

        /* Adjust word height according to zoom */
        ascent = roundf(ascent * scale);
        descent = roundf(descent * scale);

        int x = 0; /*x of bitmap*/
        int ys = 0;

        for (int codepoint = START_CODEPOINT; codepoint <= end_codepoint;
             codepoint++) {
            /**
             * Get the measurement in the horizontal direction
             * advanceWidth: Word width;
             * leftSideBearing: Left side position;
             */
            int advanceWidth = 0;
            int leftSideBearing = 0;
            stbtt_GetCodepointHMetrics(&info, codepoint, &advanceWidth,
                                       &leftSideBearing);

            /* Gets the border of a character */
            int c_x1, c_y1, c_x2, c_y2;
            stbtt_GetCodepointBitmapBox(&info, codepoint, scale, scale, &c_x1,
                                        &c_y1, &c_x2, &c_y2);

            /* Calculate the y of the bitmap (different characters have
             * different heights) */
            int y = ys + ascent + c_y1;

            /* Render character */
            int byteOffset =
                x + roundf(leftSideBearing * scale) + (y * bitmap_w);
            stbtt_MakeCodepointBitmap(&info, bitmap + byteOffset, c_x2 - c_x1,
                                      c_y2 - c_y1, bitmap_w, scale, scale,
                                      codepoint);

            /* Adjust x */
            x += FONT_SIZE;

            if (x >= bitmap_w) {
                x = 0;
                ys += FONT_SIZE;
            }
        }

        /* Save the bitmap data to the 1-channel png image */
        std::string out_filename = fmt::format("./resources/{}.png", fontname);
        stbi_write_png(out_filename.c_str(), bitmap_w, bitmap_h, 1, bitmap,
                       bitmap_w);
        //

        for (int i = 0; i < bitmap_h / 2; i++) {
            int k = bitmap_h - 1 - i;
            for (int j = 0; j < bitmap_w; j++) {
                unsigned char tmp = bitmap[i * bitmap_w + j];
                bitmap[i * bitmap_w + j] = bitmap[k * bitmap_w + j];
                bitmap[k * bitmap_w + j] = tmp;
            }
        }

        std::shared_ptr<Font> font = std::make_shared<Font>();
        font->name = fontname;
        font->size = FONT_SIZE;
        font->ascent = ascent;
        font->descent = descent;
        font->linegap = lineGap;

        std::shared_ptr<Texture> fontTexture =
            std::make_shared<Texture2D>(fontname, bitmap_w, bitmap_h);
        fontTexture->setBitmapData(bitmap);
        fontTexture->tilingFactor = 1.f;
        textureLibrary.add(fontTexture);
        font->texture = fontTexture;

        int i = 0;
        int j = 0;
        int num_rows = bitmap_h / FONT_SIZE;

        for (int codepoint = START_CODEPOINT; codepoint <= end_codepoint;
             codepoint++) {
            auto texname = fmt::format("{}_{}", fontname, codepoint);
            textureLibrary.addSubtexture(fontname, texname, i, num_rows - 1 - j,
                                         FONT_SIZE, FONT_SIZE);
            if (i >= ITEMS_PER_ROW) {
                j++;
                i = 0;
            }
            i++;
        }

        fonts[fontname] = font;

        free(fontBuffer);
        free(bitmap);
    }
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
                    // not doing it now since need to make the index
                    // correctly
                    //
                    // int idx = i * font->glyphs.size() + j;
                    // int a = font->glyphs[i].codepoint;
                    // int b = font->glyphs[j].codepoint;
                    // int kern = stbtt_GetCodepointKernAdvance(&info, a,
                    // b);
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
                .aspect = (1.f * glyph_width) / glyph_height,  //
                .height = glyph_height,                        //
                .pixels = image,                               //
                .width = glyph_width,                          //
            });
            font->glyphs.push_back(g);
            stbtt_FreeBitmap(bitmap, 0);
        }

        fonts[font->name] = font;
    }
}

static int utf8_to_utf32(const uint8_t* utf8, uint32_t* utf32, int max) {
    unsigned int c;
    int i = 0;
    --max;
    while (*utf8) {
        if (i >= max) return 0;
        if (!(*utf8 & 0x80U)) {
            utf32[i++] = *utf8++;
        } else if ((*utf8 & 0xe0U) == 0xc0U) {
            c = (*utf8++ & 0x1fU) << 6;
            if ((*utf8 & 0xc0U) != 0x80U) return 0;
            utf32[i++] = c + (*utf8++ & 0x3fU);
        } else if ((*utf8 & 0xf0U) == 0xe0U) {
            c = (*utf8++ & 0x0fU) << 12;
            if ((*utf8 & 0xc0U) != 0x80U) return 0;
            c += (*utf8++ & 0x3fU) << 6;
            if ((*utf8 & 0xc0U) != 0x80U) return 0;
            utf32[i++] = c + (*utf8++ & 0x3fU);
        } else if ((*utf8 & 0xf8U) == 0xf0U) {
            c = (*utf8++ & 0x07U) << 18;
            if ((*utf8 & 0xc0U) != 0x80U) return 0;
            c += (*utf8++ & 0x3fU) << 12;
            if ((*utf8 & 0xc0U) != 0x80U) return 0;
            c += (*utf8++ & 0x3fU) << 6;
            if ((*utf8 & 0xc0U) != 0x80U) return 0;
            c += (*utf8++ & 0x3fU);
            if ((c & 0xFFFFF800U) == 0xD800U) return 0;
            if (c >= 0x10000U) {
                c -= 0x10000U;
                if (i + 2 > max) return 0;
                utf32[i++] = 0xD800U | (0x3ffU & (c >> 10));
                utf32[i++] = 0xDC00U | (0x3ffU & (c));
            }
        } else
            return 0;
    }
    utf32[i] = 0;
    return i;
}

