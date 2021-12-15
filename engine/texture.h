
#pragma once

#include "pch.hpp"

struct Texture {
    std::string name;
    int width;
    int height;
    float tilingFactor;

    Texture()
        : name("TEXTURE_HAS_NO_NAME"), width(0), height(0), tilingFactor(1.f) {}
    Texture(const std::string &n, int w, int h)
        : name(n), width(w), height(h), tilingFactor(1.f) {}

    Texture(const Texture &tex)
        : name(tex.name),
          width(tex.width),
          height(tex.height),
          tilingFactor(tex.tilingFactor) {}
    virtual void setData(void *data) { (void)data; }

    virtual ~Texture() {}
    // its const from a C++ pov but
    // its not really const if you know what i mean
    virtual void bind(int i) const = 0;

    bool operator==(const Texture &other) const {
        return other.name == this->name;
    }
};

struct Texture2D : public Texture {
    unsigned int rendererID;

    Texture2D(const std::string &name, int w, int h) : Texture(name, w, h) {
        glGenTextures(1, &rendererID);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rendererID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    virtual void setData(void *data) override {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, data);
    }

    Texture2D(const Texture2D &other) {
        this->rendererID = other.rendererID;
        this->name = other.name;
        this->width = other.width;
        this->height = other.height;
        this->tilingFactor = other.tilingFactor;
    }

    Texture2D &operator=(Texture2D &other) {
        if (this != &other) {
            this->rendererID = other.rendererID;
            this->name = other.name;
            this->width = other.width;
            this->height = other.height;
            this->tilingFactor = other.tilingFactor;
        }
        return *this;
    }

    Texture2D(const std::string &path) : Texture() {
        log_trace(fmt::format("Loading texture: {}", path));

        name = nameFromFilePath(path);

        int w, h, channels;
        stbi_set_flip_vertically_on_load(1);
        stbi_uc *data = stbi_load(path.c_str(), &w, &h, &channels, 0);
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
        log_trace(fmt::format("texture {} has {} channels", path, channels));

        width = w;
        height = h;

        glGenTextures(1, &rendererID);
        glBindTexture(GL_TEXTURE_2D, rendererID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
                     dataFormat, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    virtual ~Texture2D() { glDeleteTextures(1, &rendererID); }
    virtual void bind(int i) const override {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, rendererID);
    }

    bool operator==(const Texture2D &other) const {
        return other.rendererID == this->rendererID;
    }
};

struct TextureLibrary {
    std::map<std::string, std::shared_ptr<Texture>> textures;

    auto size() { return textures.size(); }
    auto begin() { return textures.begin(); }
    auto end() { return textures.end(); }

    auto begin() const { return textures.begin(); }
    auto end() const { return textures.end(); }

    auto rbegin() const { return textures.rbegin(); }
    auto rend() const { return textures.rend(); }

    auto rbegin() { return textures.rbegin(); }
    auto rend() { return textures.rend(); }

    auto empty() const { return textures.empty(); }

    const std::string add(const std::shared_ptr<Texture> &texture) {
        if (textures.find(texture->name) != textures.end()) {
            log_warn(fmt::format(
                "Failed to add texture to library, texture with name "
                "{} already exists",
                texture->name));
            return "";
        }
        log_info(
            fmt::format("Adding Texture \"{}\" to our library", texture->name));
        textures[texture->name] = texture;
        return texture->name;
    }

    const std::string load(const std::string &path) {
        auto texture = std::make_shared<Texture2D>(path);
        return add(texture);
    }

    std::shared_ptr<Texture> &get(const std::string &name) {
        return textures[name];
    }
};

static TextureLibrary textureLibrary;
