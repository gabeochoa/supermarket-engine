

#pragma once
//
#include "pch.hpp"
//
#include "buffer.h"
#include "camera.h"
#include "shader.h"

// TODO if there are ever any other renderers (directx vulcan metal)
// then have to subclass this for each one
struct Renderer3D {
    struct SceneData {
        glm::mat4 viewProjection;
    };

    static SceneData* sceneData;

    static void init() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    static void begin(OrthoCamera& cam) {
        sceneData->viewProjection = cam.viewProjection;
    }
    static void end() {}

    static void clear(const glm::vec4& color) {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);  // Clear the buffers
    }

    static void submit(const std::shared_ptr<VertexArray>& vertexArray,
                       const std::shared_ptr<Shader>& shader,
                       const glm::mat4& transform = glm::mat4(1.f)) {
        shader->bind();
        shader->uploadUniformMat4("viewProjection", sceneData->viewProjection);
        shader->uploadUniformMat4("transformMatrix", transform);

        vertexArray->bind();
        Renderer3D::draw(vertexArray);
    }

    static void draw(const std::shared_ptr<VertexArray>& vertexArray) {
        glDrawElements(GL_TRIANGLES, vertexArray->indexBuffer->getCount(),
                       GL_UNSIGNED_INT, nullptr);
    }
};

struct Renderer {
    struct SceneData {
        glm::mat4 viewProjection;

        std::shared_ptr<VertexArray> vertexArray;
        ShaderLibrary shaderLibrary;
    };

    static SceneData* sceneData;

    static void init() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);

        std::shared_ptr<VertexBuffer> squareVB;
        std::shared_ptr<IndexBuffer> squareIB;
        sceneData->vertexArray.reset(VertexArray::create());

        // float squareVerts[] = {
        // 0.f, 0.f, 0.f,  //
        // 1.f, 0.f, 0.f,  //
        // 1.f, 1.f, 0.f,  //
        // 0.f, 1.f, 0.f,  //
        // };
        float squareVerts[5 * 4] = {
            0.f, 0.f, 0.f, 0.0f, 0.0f,  //
            1.f, 0.f, 0.f, 1.0f, 0.0f,  //
            1.f, 1.f, 0.f, 1.0f, 1.0f,  //
            0.f, 1.f, 0.f, 0.0f, 1.0f,  //
        };
        squareVB.reset(VertexBuffer::create(squareVerts, sizeof(squareVerts)));
        squareVB->setLayout(BufferLayout{
            {"i_pos", BufferType::Float3},
            {"i_texcoord", BufferType::Float2},
        });
        sceneData->vertexArray->addVertexBuffer(squareVB);

        unsigned int squareIs[] = {0, 1, 2, 0, 2, 3};
        squareIB.reset(IndexBuffer::create(squareIs, 6));
        sceneData->vertexArray->setIndexBuffer(squareIB);

        sceneData->shaderLibrary.load("./engine/shaders/flat.glsl");
        sceneData->shaderLibrary.load("./engine/shaders/texture.glsl");
    }

    static void resize(int width, int height) {
        glViewport(0, 0, width, height);
    }

    static void begin(OrthoCamera& cam) {
        prof(__PROFILE_FUNC__);
        sceneData->viewProjection = cam.viewProjection;

        auto flatShader = sceneData->shaderLibrary.get("flat");
        flatShader->bind();
        flatShader->uploadUniformMat4("viewProjection",
                                      sceneData->viewProjection);

        auto textureShader = sceneData->shaderLibrary.get("texture");
        textureShader->bind();
        textureShader->uploadUniformMat4("viewProjection",
                                         sceneData->viewProjection);
    }

    static void end() { prof(__PROFILE_FUNC__); }

    static void shutdown() { delete sceneData; }

    static void clear(const glm::vec4& color) {
        prof(__PROFILE_FUNC__);
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);  // Clear the buffers
    }

    static void draw(const std::shared_ptr<VertexArray>& vertexArray) {
        prof(__PROFILE_FUNC__);
        glDrawElements(GL_TRIANGLES, vertexArray->indexBuffer->getCount(),
                       GL_UNSIGNED_INT, nullptr);
    }

    static void drawQuad(const glm::vec3& position, const glm::vec2& size,
                         const glm::vec4& color) {
        prof(__PROFILE_FUNC__);
        auto transform = glm::translate(glm::mat4(1.f), position) *
                         glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        auto flatShader = sceneData->shaderLibrary.get("flat");
        flatShader->bind();
        flatShader->uploadUniformMat4("transformMatrix", transform);
        flatShader->uploadUniformFloat4("u_color", color);

        sceneData->vertexArray->bind();
        Renderer::draw(sceneData->vertexArray);
    }

    static void drawQuad(const glm::vec3& position, const glm::vec2& size,
                         const glm::vec4& color,
                         const std::shared_ptr<Texture>& texture) {
        prof(__PROFILE_FUNC__);
        auto transform = glm::translate(glm::mat4(1.f), position) *
                         glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        auto textureShader = sceneData->shaderLibrary.get("texture");
        textureShader->bind();
        textureShader->uploadUniformMat4("transformMatrix", transform);
        textureShader->uploadUniformInt("u_texture", texture->textureIndex);
        textureShader->uploadUniformFloat4("u_color", color);

        texture->bind();

        sceneData->vertexArray->bind();
        Renderer::draw(sceneData->vertexArray);
    }

    ////// ////// ////// ////// ////// ////// ////// //////
    //      the draw quads below here, just call one of the ones above
    ////// ////// ////// ////// ////// ////// ////// //////

    static void drawQuad(const glm::vec2& position, const glm::vec2& size,
                         const glm::vec4& color) {
        Renderer::drawQuad(glm::vec3{position.x, position.y, 0.f}, size, color);
    }

    static void drawQuad(const glm::vec2& position, const glm::vec2& size,
                         const glm::vec4& color,
                         const std::shared_ptr<Texture>& texture) {
        Renderer::drawQuad(glm::vec3{position.x, position.y, 0.f}, size, color,
                           texture);
    }
};
