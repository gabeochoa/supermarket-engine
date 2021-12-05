
#pragma once

#include "pch.hpp"

struct Texture {
    int width;
    int height;

    Texture() : width(0), height(0) {}

    virtual ~Texture() {}
    // its const from a C++ pov but
    // its not really const if you know what i mean
    virtual void bind(int slot = 0) const = 0;
};

struct Texture2D : public Texture {
    unsigned int rendererID;

    Texture2D(const std::string& path) : Texture() {
        log_info(fmt::format("Loading texture: {}", path));

        int w, h, channels;
        stbi_set_flip_vertically_on_load(1);
        stbi_uc* data = stbi_load(path.c_str(), &w, &h, &channels, 0);
        M_ASSERT(data, fmt::format("Failed to load texture2d: {}", path));

        GLenum internalFormat = 0, dataFormat = 0;
        if (channels == 4) {
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
        } else if (channels == 3) {
            internalFormat = GL_RGB8;
            dataFormat = GL_RGB;
        }
        M_ASSERT(internalFormat,
                 fmt::format("image format not supported: {}", channels));
        log_info(fmt::format("texture {} has {} channels", path, channels));

        width = w;
        height = h;

        glGenTextures(1, &rendererID);
        // glBindTexture(GL_TEXTURE_2D, rendererID);

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
                     dataFormat, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        stbi_image_free(data);
    }

    virtual ~Texture2D() { glDeleteTextures(1, &rendererID); }
    virtual void bind(int slot = 0) const override {
        glBindTexture(slot, rendererID);
    }
};

