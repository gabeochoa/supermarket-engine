#pragma once

#include <memory>
#include <string>
#include <vector>

#include "mrender/renderer.h"
#include "mrender/texture.h"

namespace mrender {

struct InitConfig {
    bool enableLineSmoothing = true;
    bool enablePolygonBatch = true;
};

struct TextureHandle {
    std::shared_ptr<Texture> texture;
    explicit operator bool() const { return static_cast<bool>(texture); }
    std::string name() const { return texture ? texture->name : std::string(); }
};

void Init(const InitConfig& config = {});
void Shutdown();

void BeginMode2D(OrthoCamera& camera);
void BeginMode3D(FreeCamera& camera);
void EndDrawing();
void ResizeViewport(int width, int height);

void ClearBackground(const glm::vec4& color);

void DrawQuad(const glm::vec2& position, const glm::vec2& size,
              const glm::vec4& color,
              const std::string& textureName = DEFAULT_TEX);
void DrawQuad(const glm::mat4& transform, const glm::vec4& color,
              const std::string& textureName = DEFAULT_TEX);
void DrawQuadRotated(const glm::vec2& position, const glm::vec2& size,
                     float angleRadians, const glm::vec4& color,
                     const std::string& textureName = DEFAULT_TEX);
void DrawLine(const glm::vec2& start, const glm::vec2& end,
              const glm::vec4& color);
void DrawPolygon(const std::vector<glm::vec2>& points,
                 const glm::vec4& color);

TextureHandle LoadTexture(const std::string& path);
TextureHandle LoadTextureFromHandle(const std::string& name);
void UnloadTexture(const TextureHandle& handle);

void DrawTexture(const TextureHandle& handle, const glm::vec2& position,
                 const glm::vec2& size, const glm::vec4& tint = {1.f, 1.f, 1.f, 1.f});

}  // namespace mrender
