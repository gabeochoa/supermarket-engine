

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
static const int MAX_TEX = 16;
struct Renderer {
    struct QuadVert {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 texcoord;
        float texindex;
        float tilingfactor;
    };

    struct SceneData {
        // Max per draw call
        const int MAX_QUADS = 1000;
        const int MAX_VERTS = MAX_QUADS * 4;
        const int MAX_IND = MAX_QUADS * 6;

        std::shared_ptr<VertexArray> quadVA;
        std::shared_ptr<VertexBuffer> quadVB;

        int quadIndexCount = 0;
        QuadVert* qvbufferstart = nullptr;
        QuadVert* qvbufferptr = nullptr;

        glm::mat4 viewProjection;

        ShaderLibrary shaderLibrary;
        std::array<std::shared_ptr<Texture>, MAX_TEX> textureSlots;
        int nextTexSlot = 1;  // 0 will be white
    };

    static SceneData* sceneData;

    // TODO lets add something similar for shaders
    static void addTexture(const std::string& filepath, float tiling = 1.f) {
        if ((int)textureLibrary.size() >= MAX_TEX) {
            log_error("Cant have more than MAX TEX textures (16)");
            return;
        }
        auto tex = textureLibrary.load(filepath);
        tex->tilingFactor = tiling;
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
        textureLibrary.add(whiteTexture);
    }

    static void init() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);

        init_default_shaders();
        init_default_textures();

        std::shared_ptr<VertexBuffer> squareVB;
        std::shared_ptr<IndexBuffer> squareIB;
        std::shared_ptr<IndexBuffer> quadIB;
        sceneData->quadVA.reset(VertexArray::create());

        sceneData->quadVB.reset(
            VertexBuffer::create(sceneData->MAX_VERTS * sizeof(QuadVert)));
        sceneData->quadVB->setLayout(BufferLayout{
            {"i_pos", BufferType::Float3},
            {"i_color", BufferType::Float4},
            {"i_texcoord", BufferType::Float2},
            {"i_texindex", BufferType::Float},
            {"i_tilingfactor", BufferType::Float},
        });
        sceneData->quadVA->addVertexBuffer(sceneData->quadVB);

        sceneData->qvbufferstart = new QuadVert[sceneData->MAX_VERTS];

        uint32_t* quadIndices = new uint32_t[sceneData->MAX_IND];
        uint32_t offset = 0;

        for (int i = 0; i < sceneData->MAX_IND; i += 6) {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4;
        }

        quadIB.reset(IndexBuffer::create(quadIndices, sceneData->MAX_IND));
        sceneData->quadVA->setIndexBuffer(quadIB);
        delete[] quadIndices;

        std::array<int, MAX_TEX> samples = {0};
        for (size_t i = 0; i < MAX_TEX; i++) {
            samples[(int)i] = (int)i;
        }
        auto textureShader = sceneData->shaderLibrary.get("texture");
        textureShader->bind();
        textureShader->uploadUniformIntArray("u_textures", samples.data(),
                                             MAX_TEX);

        // float squareVerts[5 * 4] = {
        // 0.f, 0.f, 0.f, 0.0f, 0.0f,  //
        // 1.f, 0.f, 0.f, 1.0f, 0.0f,  //
        // 1.f, 1.f, 0.f, 1.0f, 1.0f,  //
        // 0.f, 1.f, 0.f, 0.0f, 1.0f,  //
        // };
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
    }

    static void resize(int width, int height) {
        glViewport(0, 0, width, height);
    }

    static void shutdown() {
        delete[] sceneData->qvbufferstart;
        delete sceneData;
    }

    static void clear(const glm::vec4& color) {
        prof(__PROFILE_FUNC__);
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);  // Clear the buffers
    }

    static void draw(const std::shared_ptr<VertexArray>& vertexArray,
                     int indexCount = 0) {
        prof(__PROFILE_FUNC__);
        int count =
            indexCount ? indexCount : vertexArray->indexBuffer->getCount();
        vertexArray->bind();
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    static void begin(OrthoCamera& cam) {
        prof(__PROFILE_FUNC__);
        sceneData->viewProjection = cam.viewProjection;

        auto textureShader = sceneData->shaderLibrary.get("texture");
        textureShader->bind();
        textureShader->uploadUniformMat4("viewProjection",
                                         sceneData->viewProjection);

        start_batch();
    }

    static void end() {
        prof(__PROFILE_FUNC__);
        flush();
    }

    static void start_batch() {
        sceneData->textureSlots[0] = textureLibrary.get("white");
        sceneData->quadIndexCount = 0;
        sceneData->qvbufferptr = sceneData->qvbufferstart;
        sceneData->nextTexSlot = 1;
    }

    static void next_batch() {
        flush();
        start_batch();
    }

    static void flush() {
        if (sceneData->quadIndexCount == 0) return;  // Nothing to draw
        uint32_t dataSize = (uint32_t)((uint8_t*)sceneData->qvbufferptr -
                                       (uint8_t*)sceneData->qvbufferstart);
        sceneData->quadVB->setData(sceneData->qvbufferstart, dataSize);

        for (int i = 0; i < sceneData->nextTexSlot; i++)
            sceneData->textureSlots[i]->bind(i);

        auto textureShader = sceneData->shaderLibrary.get("texture");
        textureShader->bind();

        draw(sceneData->quadVA, sceneData->quadIndexCount);
    }

    static void drawQuad(const glm::mat4& transform, const glm::vec4& color,
                         const std::string& textureName = DEFAULT_TEX) {
        (void)textureName;
        const std::array<glm::vec2, 4> textureCoords = {
            {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}}};

        const std::array<glm::vec4, 4> vertexCoords = {{
            {-0.5f, -0.5f, 0.0f, 1.0f},
            {0.5f, -0.5f, 0.0f, 1.0f},
            {0.5f, 0.5f, 0.0f, 1.0f},
            {-0.5f, 0.5f, 0.0f, 1.0f},
        }};

        if (sceneData->quadIndexCount >= sceneData->MAX_IND) {
            next_batch();
        }

        auto texture = textureLibrary.get(textureName);
        int textureIndex = 0;
        if (textureName != DEFAULT_TEX && texture) {
            for (int i = 1; i < sceneData->nextTexSlot; i++) {
                if (*(sceneData->textureSlots[i]) == *texture) {
                    textureIndex = i;
                    break;
                }
            }
            if (textureIndex == 0) {
                if (sceneData->nextTexSlot >= MAX_TEX) {
                    next_batch();
                }
                textureIndex = sceneData->nextTexSlot;
                sceneData->textureSlots[textureIndex] = texture;
                sceneData->nextTexSlot++;
            }
        } else {
            texture = textureLibrary.get(DEFAULT_TEX);
        }
        // else use 0 which is white texture

        for (size_t i = 0; i < 4; i++) {
            sceneData->qvbufferptr->position = transform * vertexCoords[i];
            sceneData->qvbufferptr->color = color;
            sceneData->qvbufferptr->texcoord = textureCoords[i];
            sceneData->qvbufferptr->texindex = textureIndex;
            sceneData->qvbufferptr->tilingfactor = texture->tilingFactor;
            // sceneData->qvbufferptr->entityID = entityID;
            sceneData->qvbufferptr++;
        }
        sceneData->quadIndexCount += 6;
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
