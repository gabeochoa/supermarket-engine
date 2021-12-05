

#pragma once
#include "buffer.h"
#include "pch.hpp"

struct Renderer {
    // TODO if there are ever any other renderers (directx vulcan metal)
    // then have to subclass this for each one

    static void begin() {}
    static void end() {}

    static void clear(const glm::vec4& color) {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);  // Clear the buffers
    }

    static void submit(const std::shared_ptr<VertexArray>& vertexArray) {
        vertexArray->bind();
        Renderer::draw(vertexArray);
    }

    static void draw(const std::shared_ptr<VertexArray>& vertexArray) {
        glDrawArrays(GL_TRIANGLES, 0, vertexArray->indexBuffer->getCount());
    }
};
