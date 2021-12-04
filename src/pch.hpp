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
#include <functional>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <vector>

#define FMT_HEADER_ONLY
#include "fmt/format.h"

// All of these files should not rely on any other header
// if they do we will run into duplicate symbol issues
#include "log.h"
#include "strutil.h"
