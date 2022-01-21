
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
    bool temporary = false;

    Texture();
    Texture(const std::string &n, int w, int h);
    Texture(const Texture &tex);
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

    Texture2D(const std::string &name, int w, int h);

    virtual void setData(void *data) override;
    virtual void setBitmapData(void *data) override;
    Texture2D(const Texture2D &other);
    Texture2D &operator=(Texture2D &other);
    Texture2D(const std::string &path);
    virtual ~Texture2D();
    virtual void bind(int i) const override;
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

    // TODO think about reconsindering the number of max temporary;
    // minimum we can support is 2 but we'd have lots of thrash constantly O(n)
    // to find it in the map ( why not 1? because it would be better in that
    // case to not even put it in the map since there would be no need to find
    // someone to evict )
    //
    // I tried keeping a totally separate list of temps but had issues with
    // rendering, it seemed liked the shader wasnt able to find them so we are
    // colocating them with the normal textures
    int numTemporaryTextures = 0;
    const int maxTempTextures = 20;

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
        log_trace("Adding Texture \"{}\" to our library", texture->name);
        if (texture->temporary) {
            numTemporaryTextures++;
        }

        if (numTemporaryTextures > maxTempTextures) {
            std::string name = "";
            for (auto i = textures.begin(), last = textures.end(); i != last;) {
                if (i->second->temporary) {
                    numTemporaryTextures--;
                    name = i->second->name;
                    log_trace(
                        "Evicting Temporary Texture \"{}\" from our library",
                        name);
                    i = textures.erase(i);
                } else {
                    ++i;
                }
            }
        }

        textures[texture->name] = texture;
        return texture->name;
    }

    const std::string load(const std::string &path) {
        auto texture = std::make_shared<Texture2D>(path);
        return add(texture);
    }

    std::shared_ptr<Texture> get(const std::string &name) {
        return textures[name];
    }

    bool hasMatchingTexture(const std::string &name) {
        return (textures.find(name) != textures.end());
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
        log_trace("Adding subtexture \"{}\" to our library", name);
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
