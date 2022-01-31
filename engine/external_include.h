#pragma once
#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#pragma clang diagnostic ignored "-Wdeprecated-volatile"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif

#ifdef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#endif

#ifdef WIN32
#include "../windows/glew-2.1.0/include/GL/glew.h"
#include "../windows/glfw-3.3.6.bin.WIN64/include/GLFW/glfw3.h"
#include "../vendor/glm/glm/glm.hpp"
#include "../vendor/glm/glm/gtc/random.hpp"
#include "../vendor/glm/glm/gtc/type_ptr.hpp"
#include "../vendor/glm/glm/gtc/matrix_transform.hpp"
#include <experimental/filesystem>
#else
#include <GL/glew.h>
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#endif

// //

#include <cstdint>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Vendor stuff
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "../vendor/stb_image.h"
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "../vendor/stb_truetype.h"
#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../vendor/stb_image_write.h"
#define FMT_HEADER_ONLY
#include "../vendor/fmt/format.h"
#include "../vendor/fmt/ostream.h"
// this is needed for wstring printing
#include "../vendor/fmt/xchar.h"
#define GLT_IMPLEMENTATION
#include "../vendor/glText.h"
#include "../vendor/portable-file-dialogs.h"

#define INCBIN_SILENCE_BITCODE_WARNING
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX g_
#include "../vendor/incbin/incbin.h"
/* Usage: INCBIN(<<LABLE>>, <<FILE>>)
 *
 * Symbols defined by INCBIN
 * ------------------------------------------
 *  const unsigned char        g_asset_data[]  // g_<<LABLE>>_data
 *  const unsigned char* const g_asset_end;    // g_<<LABEL>>_end
 *  const unsinged int         g_asset_size;   // g_<<LABEL>>_size
 */

#ifdef __APPLE__
#pragma clang diagnostic pop
#else
#pragma enable_warn
#endif

#ifdef WIN32
#pragma GCC diagnostic pop
#endif

