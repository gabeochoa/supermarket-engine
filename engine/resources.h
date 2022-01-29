
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
        init();
    }

    void init(){
        fonts = fmt::format("{}/fonts", folder);
        keybindings = fmt::format("{}/keybindings.ini", folder);
        default_shaders = fmt::format("{}/shaders", folder);
        shaders = fmt::format("{}/shaders", folder);
        log_trace("Will be loading resources from {}", folder);
        log_trace("Font folder {}", fonts);
        log_trace("Shaders folder {}", shaders);
        log_trace("Keybindings file: {}", keybindings);
    }
};

static ResourceLocations resourceLocations_DO_NOT_USE;

inline ResourceLocations& getResourceLocations() {
    return resourceLocations_DO_NOT_USE;
}

