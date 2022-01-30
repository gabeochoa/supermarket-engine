
#pragma once

//
#include <stdio.h>
#include <stdlib.h>

#include <cstring>
#include <string_view>

#include "pch.hpp"
//
#include "resources.h"
#include "strutil.h"

const int START_CODEPOINT = 32;
const int END_CODEPOINT = 30922;
const int FONT_SIZE = 64 * 2;
const int ITEMS_PER_ROW = 50;
const int NUM_LETTERS = END_CODEPOINT - START_CODEPOINT;

// Note: using a macro cause const char* didnt play nice with incbin
#define DEFAULT_FONT "./resources/fonts/Karmina-Regular.ttf"
#define DEFAULT_CJK_FONT "./resources/fonts/Sazanami-Hanazono-Mincho.ttf"

std::shared_ptr<Texture> fetch_texture_for_phrase(
    const std::wstring phrase, const char* fontname = "default",
    bool temporary = false);
