#pragma once

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#endif
#include <GL/glew.h>
/* Ask for an OpenGL Core Context */
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include <cstdint>
#include <fstream>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Vendor stuff

#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#else
#pragma disable_warn
#endif

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "../vendor/stb_image.h"
#define FMT_HEADER_ONLY
#include "../vendor/fmt/format.h"
#define GLT_IMPLEMENTATION
#include "../vendor/glText.h"

#ifdef __APPLE__
#pragma clang diagnostic pop
#else
#pragma enable_warn
#endif

// All of these files should not rely on any other header
// if they do we will run into duplicate symbol issues
#include "log.h"
#include "strutil.h"
#include "texture.h"
#include "time.h"

