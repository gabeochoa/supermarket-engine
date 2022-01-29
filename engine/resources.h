
#pragma once

#include "globals.h"
#include "pch.hpp"

struct ResourceLocations {
    std::string folder;
    std::string fonts;
    std::string keybindings;
    // TODO consider just keeping these strings inside the codebase instead of
    // files?
    std::string default_shaders;
    std::string shaders;

    ResourceLocations() {
        folder = "./resources";
        fonts = fmt::format("{}/fonts", folder);
        keybindings = fmt::format("{}/keybindings.ini", folder);
        default_shaders = fmt::format("{}/shaders", folder);
        shaders = fmt::format("{}/shaders", folder);
    }
};

static ResourceLocations resourceLocations_DO_NOT_USE;

inline ResourceLocations& getResourceLocations() {
    return resourceLocations_DO_NOT_USE;
}

