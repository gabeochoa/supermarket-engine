#include "mrender/mrender.h"

namespace mrender {

static InitConfig g_config;

void Init(const InitConfig& config) {
    g_config = config;
    Renderer::init();
    if (!g_config.enableLineSmoothing) {
        glDisable(GL_LINE_SMOOTH);
    }
    if (!g_config.enablePolygonBatch) {
        // Currently nothing to disable specifically but keep hook for future.
    }
}

void Shutdown() { Renderer::shutdown(); }

void BeginMode2D(OrthoCamera& camera) { Renderer::begin(camera); }
void BeginMode3D(FreeCamera& camera) { Renderer::begin(camera); }
void EndDrawing() { Renderer::end(); }
void ResizeViewport(int width, int height) { Renderer::resize(width, height); }

void ClearBackground(const glm::vec4& color) { Renderer::clear(color); }

void DrawQuad(const glm::vec2& position, const glm::vec2& size,
              const glm::vec4& color, const std::string& textureName) {
    Renderer::drawQuad(position, size, color, textureName);
}

void DrawQuad(const glm::mat4& transform, const glm::vec4& color,
              const std::string& textureName) {
    Renderer::drawQuad(transform, color, textureName);
}

void DrawQuadRotated(const glm::vec2& position, const glm::vec2& size,
                     float angleRadians, const glm::vec4& color,
                     const std::string& textureName) {
    Renderer::drawQuadRotated(position, size, angleRadians, color,
                              textureName);
}

void DrawLine(const glm::vec2& start, const glm::vec2& end,
              const glm::vec4& color) {
    Renderer::drawLine(start, end, color);
}

void DrawPolygon(const std::vector<glm::vec2>& points,
                 const glm::vec4& color) {
    Renderer::drawPolygon(points, color);
}

TextureHandle LoadTexture(const std::string& path) {
    TextureHandle handle;
    auto name = TextureLibrary::get().load(path);
    if (!name.empty()) {
        handle.texture = TextureLibrary::get().get(name);
    }
    return handle;
}

TextureHandle LoadTextureFromHandle(const std::string& name) {
    TextureHandle handle;
    handle.texture = TextureLibrary::get().get(name);
    return handle;
}

void UnloadTexture(const TextureHandle& handle) {
    if (!handle.texture) return;
    TextureLibrary::get().remove(handle.texture->name);
}

void DrawTexture(const TextureHandle& handle, const glm::vec2& position,
                 const glm::vec2& size, const glm::vec4& tint) {
    if (!handle.texture) {
        Renderer::drawQuad(position, size, tint);
        return;
    }
    Renderer::drawQuad(position, size, tint, handle.texture->name);
}

}  // namespace mrender
