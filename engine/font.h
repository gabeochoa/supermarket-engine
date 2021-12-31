
#pragma once

//
#include <stdio.h>
#include <stdlib.h>

#include <string_view>

#include "pch.hpp"
//
#include "../engine/strutil.h"

const int START_CODEPOINT = 32;
const int END_CODEPOINT = 30922;
const int FONT_SIZE = 64 * 2;
const int ITEMS_PER_ROW = 50;
const int NUM_LETTERS = END_CODEPOINT - START_CODEPOINT;

static std::shared_ptr<Texture> fetch_texture_for_phrase(
    const char* fontname, const std::wstring phrase, bool temporary = false) {
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
