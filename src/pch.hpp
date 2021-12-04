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
#include <glm/glm.hpp>
#include <iostream>
#include <string>

#define FMT_HEADER_ONLY
#include "event.hpp"
#include "fmt/format.h"
#include "log.h"
