

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

static const char* DEFAULT_TEX = "white";
static int TEXTURE_INDEX = 1;
struct Renderer {
    struct QuadVert {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 texcoord;
    };

    struct SceneData {
        // Max per draw call
        const int MAX_QUADS = 10000;
        const int MAX_VERTS = MAX_QUADS * 4;
        const int MAX_IND = MAX_QUADS * 6;

        int quadIndexCount = 0;

        QuadVert* qvbufferstart = nullptr;
        QuadVert* qvbufferptr = nullptr;

        std::shared_ptr<VertexArray> quadVA;
        std::shared_ptr<VertexBuffer> quadVB;

        glm::mat4 viewProjection;
        ShaderLibrary shaderLibrary;
        TextureLibrary textureLibrary;
    };

    static SceneData* sceneData;

    // TODO lets add something similar for shaders
    static void addTexture(const std::string& filepath, float tiling = 1.f) {
        auto tex = sceneData->textureLibrary.load(filepath, TEXTURE_INDEX);
        tex->tilingFactor = tiling;
        TEXTURE_INDEX++;
    }

    static void init_default_shaders() {
        sceneData->shaderLibrary.load("./engine/shaders/flat.glsl");
        sceneData->shaderLibrary.load("./engine/shaders/texture.glsl");
    }

    static void init_default_textures() {
        std::shared_ptr<Texture> whiteTexture =
            std::make_shared<Texture2D>("white", 1, 1, 0);
        unsigned int data = 0xffffffff;
        whiteTexture->setData(&data);
        sceneData->textureLibrary.add(whiteTexture);
    }

    static void init() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);

        init_default_shaders();
        init_default_textures();

        std::shared_ptr<VertexBuffer> squareVB;
        std::shared_ptr<IndexBuffer> squareIB;
        sceneData->quadVA.reset(VertexArray::create());

        float squareVerts[5 * 4] = {
            0.f, 0.f, 0.f, 0.0f, 0.0f,  //
            1.f, 0.f, 0.f, 1.0f, 0.0f,  //
            1.f, 1.f, 0.f, 1.0f, 1.0f,  //
            0.f, 1.f, 0.f, 0.0f, 1.0f,  //
        };
        // float sheetWidth = 918.f;
        // float sheetHeight = 203.f;
        // float spriteWidth = 17.f;
        // float spriteHeight = 17.f;
        //
        // int x = 0;
        // int y = 10;
        //
        // float squareVerts[5 * 4] = {
        // 0.f,
        // 0.f,
        // 0.f,  //
        // ((x * spriteWidth) / sheetWidth),
        // ((y * spriteHeight) / sheetHeight),
        // //
        // 1.f,
        // 0.f,
        // 0.f,  //
        // (((x + 1) * spriteWidth) / sheetWidth),
        // ((y * spriteHeight) / sheetHeight),
        // 1.f,
        // 1.f,
        // 0.f,  //
        // (((x + 1) * spriteWidth) / sheetWidth),
        // (((y + 1) * spriteHeight) / sheetHeight),
        // 0.f,
        // 1.f,
        // 0.f,  //
        // ((x * spriteWidth) / sheetWidth),
        // (((y + 1) * spriteHeight) / sheetHeight),
        // };
        squareVB.reset(VertexBuffer::create(squareVerts, sizeof(squareVerts)));
        squareVB->setLayout(BufferLayout{
            {"i_pos", BufferType::Float3},
            // {"i_color", BufferType::Float4},
            {"i_texcoord", BufferType::Float2},
        });
        sceneData->quadVA->addVertexBuffer(squareVB);

        unsigned int squareIs[] = {0, 1, 2, 0, 2, 3};
        squareIB.reset(IndexBuffer::create(squareIs, 6));
        sceneData->quadVA->setIndexBuffer(squareIB);
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
    static void drawQuad(const glm::mat4& transform, const glm::vec4& color,
                         const std::string& textureName = DEFAULT_TEX) {
        auto texture = sceneData->textureLibrary.get(textureName);
        if (texture == nullptr) {
            auto flatShader = sceneData->shaderLibrary.get("flat");
            flatShader->bind();
            flatShader->uploadUniformMat4("transformMatrix", transform);
            flatShader->uploadUniformFloat4("u_color", color);
        } else {
            auto textureShader = sceneData->shaderLibrary.get("texture");
            textureShader->bind();
            textureShader->uploadUniformMat4("transformMatrix", transform);
            textureShader->uploadUniformInt("u_texture", texture->textureIndex);
            textureShader->uploadUniformFloat4("u_color", color);
            // TODO if we end up wanting this then obv have to expose it
            // through function param
            textureShader->uploadUniformFloat("f_tiling",
                                              texture->tilingFactor);
            texture->bind();
        }

        sceneData->quadVA->bind();
        Renderer::draw(sceneData->quadVA);
    }

    static void drawQuad(const glm::vec3& position, const glm::vec2& size,
                         const glm::vec4& color,
                         const std::string& textureName = DEFAULT_TEX) {
        prof(__PROFILE_FUNC__);
        auto transform = glm::translate(glm::mat4(1.f), position) *
                         glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        drawQuad(transform, color, textureName);
    }

    static void drawQuadRotated(const glm::vec3& position,
                                const glm::vec2& size, float angleInRad,
                                const glm::vec4& color,
                                const std::string& textureName = DEFAULT_TEX) {
        prof(__PROFILE_FUNC__);

        auto transform =
            glm::translate(glm::mat4(1.f), position) *
            glm::rotate(glm::mat4(1.f), angleInRad, {0.0f, 0.0f, 1.f}) *
            glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        drawQuad(transform, color, textureName);
    }

    ////// ////// ////// ////// ////// ////// ////// //////
    //      the draw quads below here, just call one of the ones above
    ////// ////// ////// ////// ////// ////// ////// //////

    static void drawQuad(const glm::vec2& position, const glm::vec2& size,
                         const glm::vec4& color,
                         const std::string& textureName = DEFAULT_TEX) {
        Renderer::drawQuad(glm::vec3{position.x, position.y, 0.f}, size, color,
                           textureName);
    }

    static void drawQuadRotated(const glm::vec2& position,
                                const glm::vec2& size, float angleInRad,
                                const glm::vec4& color,
                                const std::string& textureName = DEFAULT_TEX) {
        Renderer::drawQuadRotated(glm::vec3{position.x, position.y, 0.f}, size,
                                  angleInRad, color, textureName);
    }
};
