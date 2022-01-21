

#include "texture.h"


Texture::Texture()
    : name("TEXTURE_HAS_NO_NAME"), width(0), height(0), tilingFactor(1.f) {}

Texture::Texture(const std::string &n, int w, int h)
    : name(n), width(w), height(h), tilingFactor(1.f) {}

Texture::Texture(const Texture &tex)
    : name(tex.name),
      width(tex.width),
      height(tex.height),
      tilingFactor(tex.tilingFactor) {}

Texture2D::Texture2D(const std::string &name, int w, int h)
    : Texture(name, w, h) {
    glGenTextures(1, &rendererID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rendererID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Texture2D::setData(void *data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, data);
}

void Texture2D::setBitmapData(void *data) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED,
                 GL_UNSIGNED_BYTE, data);
}

Texture2D::Texture2D(const Texture2D &other) {
    this->rendererID = other.rendererID;
    this->name = other.name;
    this->width = other.width;
    this->height = other.height;
    this->tilingFactor = other.tilingFactor;
}

Texture2D &Texture2D::operator=(Texture2D &other) {
    if (this != &other) {
        this->rendererID = other.rendererID;
        this->name = other.name;
        this->width = other.width;
        this->height = other.height;
        this->tilingFactor = other.tilingFactor;
    }
    return *this;
}

Texture2D::Texture2D(const std::string &path) : Texture() {
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
    log_trace("texture {} has {} channels", path, channels);
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

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat,
                 GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
}

Texture2D::~Texture2D() { glDeleteTextures(1, &rendererID); }
void Texture2D::bind(int i) const {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, rendererID);
}

