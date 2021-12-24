
#pragma once

#include <array>
#include <string>

#include "external_include.h"
#include "log.h"
#include "strutil.h"

struct Texture {
    const std::array<glm::vec2, 4> textureCoords = {
        {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}}};

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
    virtual void setBitmapData(void *data) { (void)data; }

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

    virtual void setBitmapData(void *data) override {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED,
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
        log_trace("Loading texture: {}", path);

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
        } else if (channels == 1) {
            internalFormat = GL_RED;
            dataFormat = GL_RED;
        }
        log_info("texture {} has {} channels", path, channels);
        M_ASSERT(internalFormat, "image format not supported: {}", channels);

        this->width = w;
        this->height = h;

        glGenTextures(1, &rendererID);
        glBindTexture(GL_TEXTURE_2D, rendererID);

        if (channels == 1) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);
        }

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

struct Subtexture {
    std::shared_ptr<Texture> texture;
    std::array<glm::vec2, 4> textureCoords;

    Subtexture(const std::shared_ptr<Texture> &tex, const glm::vec2 &min,
               const glm::vec2 &max)
        : texture(tex) {
        textureCoords[0] = {min.x, min.y};
        textureCoords[1] = {max.x, min.y};
        textureCoords[2] = {max.x, max.y};
        textureCoords[3] = {min.x, max.y};
    }
};

struct TextureLibrary {
    std::map<std::string, std::shared_ptr<Texture>> textures;
    std::map<std::string, std::shared_ptr<Subtexture>> subtextures;

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
            log_warn(
                "Failed to add texture to library, texture with name {} "
                "already exists",
                texture->name);
            return "";
        }
        log_info("Adding Texture \"{}\" to our library", texture->name);
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

    bool hasMatchingTexture(const std::string &name) {
        return textures.find(name) != textures.end();
    }

    bool hasMatchingSubtexture(const std::string &name) {
        return subtextures.find(name) != subtextures.end();
    }

    // -1 if not found, 0 if texture, 1 if subtexture
    int isTextureOrSubtexture(const std::string &textureName) {
        int textureStatus = -1;

        auto textureIt = textures.find(textureName);
        auto subtextureIt = subtextures.find(textureName);

        // is this a texture?
        if (textureIt == textures.end()) {
            // not a valid texture, is it a subtexture?
            if (subtextureIt == subtextures.end()) {
                // pass -1
            } else {
                textureStatus = 1;  // is subtexture
            }
        } else {
            textureStatus = 0;  // is texture
        }
        return textureStatus;
    }

    void addSubtextureMinMax(const std::shared_ptr<Texture> &texture,
                             const std::string &name, glm::vec2 min,
                             glm::vec2 max) {
        log_info("Adding subtexture \"{}\" to our library", name);
        subtextures[name] = std::make_shared<Subtexture>(texture, min, max);
    }

    void addSubtexture(const std::string &textureName, const std::string &name,
                       float x, float y, float spriteWidth,
                       float spriteHeight) {
        auto textureIt = textures.find(textureName);
        if (textureIt == textures.end()) {
            log_warn(
                "Failed to add subtexture to library, texture with name "
                "{} was not found",
                textureName);
            return;
        }

        // This makes the renderer code easier because we can just assume theres
        // no conflicts. In order to fix this we would need a way for the user
        // to specify if they want to drawQuad or drawQuadSubtexture
        // or maybe we just have those two functions and they handle tiebreaks
        // differently...
        if (textures.find(name) != textures.end()) {
            log_warn(
                "Failed to add subtexture to library, this name '{}' is "
                "already being used for a texture, and cant be reused for a "
                "subtexture",
                name);
            return;
        }

        if (subtextures.find(name) != subtextures.end()) {
            log_warn(
                "Failed to add subtexture to library, subtexture with name "
                "{} already exists",
                name);
            return;
        }

        auto texture = textureIt->second;

        glm::vec2 min = {
            (x * spriteWidth) / texture->width,
            (y * spriteHeight) / texture->height,
        };
        glm::vec2 max = {
            ((x + 1) * spriteWidth) / texture->width,
            ((y + 1) * spriteHeight) / texture->height,
        };

        addSubtextureMinMax(texture, name, min, max);
    }

    std::shared_ptr<Subtexture> &getSubtexture(const std::string &name) {
        return subtextures[name];
    }
};

static TextureLibrary textureLibrary;
