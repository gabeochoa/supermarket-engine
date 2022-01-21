

#pragma once
//
#include "pch.hpp"
//
#include "buffer.h"
#include "camera.h"
#include "shader.h"
#include "texture.h" 

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
static const glm::mat4 imat(1.f);
struct Renderer {
    struct QuadVert {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 texcoord;
        float texindex;
        // float tilingfactor;
    };

    struct LineVert {
        glm::vec3 position;
        glm::vec4 color;
    };

    struct PolyVert {
        glm::vec3 position;
        glm::vec4 color;
    };

    struct Statistics {
        std::array<float, 100> renderTimes;
        int drawCalls = 0;
        int quadCount = 0;
        int textureCount = 0;
        size_t frameCount = 0;
        float frameBeginTime = 0.f;
        float totalFrameTime = 0.f;

        void reset() {
            drawCalls = 0;
            quadCount = 0;
            textureCount = 0;
        }

        void begin() { frameBeginTime = (float)glfwGetTime(); }

        void end() {
            auto endt = (float)glfwGetTime();
            renderTimes[frameCount] = endt - frameBeginTime;
            totalFrameTime +=
                renderTimes[frameCount] -
                renderTimes[(frameCount + 1) % renderTimes.size()];
            frameCount += 1;
            if (frameCount >= renderTimes.size()) {
                frameCount = 0;
            }
        }
    };

    static Statistics stats;

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

        std::shared_ptr<VertexArray> lineVA;
        std::shared_ptr<VertexBuffer> lineVB;

        int lineVertexCount = 0;
        LineVert* lvbufferstart = nullptr;
        LineVert* lvbufferptr = nullptr;

        std::shared_ptr<VertexArray> polyVA;
        std::shared_ptr<VertexBuffer> polyVB;

        int polyVertexCount = 0;
        PolyVert* pvbufferstart = nullptr;
        PolyVert* pvbufferptr = nullptr;

        glm::mat4 viewProjection;

        ShaderLibrary shaderLibrary;
        std::array<std::shared_ptr<Texture2D>, MAX_TEX> textureSlots;
        int nextTexSlot = 1;  // 0 will be white
    };

    static SceneData* sceneData;

    // TODO lets add something similar for shaders
    static void addTexture(const std::string& filepath) {
        auto texName = textureLibrary.load(filepath);
        textureLibrary.get(texName)->tilingFactor = 1.f;
    }

    static void addSubtexture(const std::string& textureName,
                              const std::string& name, float x, float y,
                              float spriteWidth, float spriteHeight) {
        textureLibrary.addSubtexture(textureName, name, x, y, spriteWidth,
                                     spriteHeight);
    }

    static void init_default_shaders() {
        sceneData->shaderLibrary.load("./shaders/flat.glsl");
        sceneData->shaderLibrary.load("./shaders/texture.glsl");
        sceneData->shaderLibrary.load("./shaders/line.glsl");
        sceneData->shaderLibrary.load("./shaders/poly.glsl");
    }

    static void init_default_textures() {
        std::shared_ptr<Texture> whiteTexture =
            std::make_shared<Texture2D>("white", 1, 1);
        unsigned int data = 0xffffffff;
        whiteTexture->setData(&data);
        textureLibrary.add(whiteTexture);
    }

    static void init_line_buffers() {
        std::shared_ptr<VertexBuffer> lineVB;
        std::shared_ptr<Shader> lineShader;

        sceneData->lineVA.reset(VertexArray::create());
        sceneData->lineVB.reset(
            VertexBuffer::create(sceneData->MAX_VERTS * sizeof(LineVert)));
        sceneData->lineVB->setLayout(BufferLayout{
            {"i_pos", BufferType::Float3},
            {"i_color", BufferType::Float4},
        });
        sceneData->lineVA->addVertexBuffer(sceneData->lineVB);
        sceneData->lvbufferstart = new LineVert[sceneData->MAX_VERTS];
    }

    static void init_quad_buffers() {
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
            // {"i_tilingfactor", BufferType::Float},
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
    }

    static void init_poly_buffers() {
        std::shared_ptr<VertexBuffer> squareVB;
        std::shared_ptr<IndexBuffer> squareIB;
        std::shared_ptr<IndexBuffer> polyIB;

        sceneData->polyVA.reset(VertexArray::create());
        sceneData->polyVB.reset(
            VertexBuffer::create(sceneData->MAX_VERTS * sizeof(PolyVert)));
        sceneData->polyVB->setLayout(BufferLayout{
            {"i_pos", BufferType::Float3},
            {"i_color", BufferType::Float4},
        });
        sceneData->polyVA->addVertexBuffer(sceneData->polyVB);

        sceneData->pvbufferstart = new PolyVert[sceneData->MAX_VERTS];

        uint32_t* polyIndices = new uint32_t[sceneData->MAX_IND];
        uint32_t offset = 0;

        for (int i = 0; i < sceneData->MAX_IND; i += 6) {
            polyIndices[i + 0] = offset + 0;
            polyIndices[i + 1] = offset + 1;
            polyIndices[i + 2] = offset + 2;

            polyIndices[i + 3] = offset + 2;
            polyIndices[i + 4] = offset + 3;
            polyIndices[i + 5] = offset + 0;

            offset += 4;
        }

        polyIB.reset(IndexBuffer::create(polyIndices, sceneData->MAX_IND));
        sceneData->polyVA->setIndexBuffer(polyIB);
        delete[] polyIndices;
    }

    static void init() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // glEnable(GL_DEPTH_TEST);
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        Renderer::setLineThickness(1.f);

        init_default_shaders();
        init_default_textures();

        init_quad_buffers();
        init_line_buffers();
        init_poly_buffers();

        std::array<int, MAX_TEX> samples = {0};
        for (size_t i = 0; i < MAX_TEX; i++) {
            samples[(int)i] = (int)i;
        }
        auto textureShader = sceneData->shaderLibrary.get("texture");
        textureShader->bind();
        textureShader->uploadUniformIntArray("u_textures", samples.data(),
                                             MAX_TEX);
    }

    static void resize(int width, int height) {
        glViewport(0, 0, width, height);
    }

    static void shutdown() {
        delete[] sceneData->qvbufferstart;
        delete[] sceneData->lvbufferstart;
        delete[] sceneData->pvbufferstart;
        delete sceneData;
    }

    static void clear(const glm::vec4& color) {
        prof(__PROFILE_FUNC__);
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);  // Clear the buffers
    }

    static void draw_INTERNAL(const std::shared_ptr<VertexArray>& vertexArray,
                              int indexCount = 0) {
        prof give_me_a_name(__PROFILE_FUNC__);
        int count =
            indexCount ? indexCount : vertexArray->indexBuffer->getCount();
        vertexArray->bind();
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    static void drawPoly_INTERNAL(
        const std::shared_ptr<VertexArray>& vertexArray, int indexCount = 0) {
        prof give_me_a_name(__PROFILE_FUNC__);
        int count =
            indexCount ? indexCount : vertexArray->indexBuffer->getCount();
        vertexArray->bind();
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    static void drawLines_INTERNAL(
        const std::shared_ptr<VertexArray>& vertexArray, int vertexCount) {
        prof drlns(__PROFILE_FUNC__);
        vertexArray->bind();
        glDrawArrays(GL_LINES, 0, vertexCount);
        // glDrawArrays(GL_LINE_STRIP, 0, vertexCount);
        // glDrawArrays(GL_LINE_LOOP, 0, vertexCount);
        // glDrawElements(GL_LINES, vertexCount, GL_UNSIGNED_INT, nullptr);
        // glDrawElements(GL_LINE_STRIP, vertexCount, GL_UNSIGNED_INT, nullptr);
        // glDrawElements(GL_LINE_LOOP, vertexCount, GL_UNSIGNED_INT, nullptr);
    }

    static void begin(OrthoCamera& cam) {
        prof give_me_a_name(__PROFILE_FUNC__);
        sceneData->viewProjection = cam.viewProjection;

        auto textureShader = sceneData->shaderLibrary.get("texture");
        textureShader->bind();
        textureShader->uploadUniformMat4("viewProjection",
                                         sceneData->viewProjection);

        auto polyShader = sceneData->shaderLibrary.get("poly");
        polyShader->bind();
        polyShader->uploadUniformMat4("viewProjection",
                                      sceneData->viewProjection);

        start_batch();
    }

    static void end() {
        prof give_me_a_name(__PROFILE_FUNC__);
        flush();
    }

    static void start_batch() {
        sceneData->textureSlots[0] =
            dynamic_pointer_cast<Texture2D>(textureLibrary.get("white"));
        sceneData->quadIndexCount = 0;
        sceneData->qvbufferptr = sceneData->qvbufferstart;
        sceneData->nextTexSlot = 1;

        sceneData->lineVertexCount = 0;
        sceneData->lvbufferptr = sceneData->lvbufferstart;

        sceneData->polyVertexCount = 0;
        sceneData->pvbufferptr = sceneData->pvbufferstart;
    }

    static void next_batch() {
        flush();
        start_batch();
    }

    static void flush() {
        if (sceneData->quadIndexCount) {
            uint32_t dataSize = (uint32_t)((uint8_t*)sceneData->qvbufferptr -
                                           (uint8_t*)sceneData->qvbufferstart);
            sceneData->quadVB->setData(sceneData->qvbufferstart, dataSize);

            for (int i = 0; i < sceneData->nextTexSlot; i++)
                sceneData->textureSlots[i]->bind(i);

            sceneData->shaderLibrary.get("texture")->bind();

            draw_INTERNAL(sceneData->quadVA, sceneData->quadIndexCount);
            stats.drawCalls++;
        }

        if (sceneData->lineVertexCount) {
            uint32_t dataSize = (uint32_t)((uint8_t*)sceneData->lvbufferptr -
                                           (uint8_t*)sceneData->lvbufferstart);
            sceneData->lineVB->setData(sceneData->lvbufferstart, dataSize);
            sceneData->shaderLibrary.get("line")->bind();
            drawLines_INTERNAL(sceneData->lineVA, sceneData->lineVertexCount);
            stats.drawCalls++;
        }

        if (sceneData->polyVertexCount) {
            uint32_t dataSize = (uint32_t)((uint8_t*)sceneData->pvbufferptr -
                                           (uint8_t*)sceneData->pvbufferstart);
            sceneData->polyVB->setData(sceneData->pvbufferstart, dataSize);
            sceneData->shaderLibrary.get("poly")->bind();
            drawPoly_INTERNAL(sceneData->polyVA, sceneData->polyVertexCount);
            stats.drawCalls++;
        }

        //
    }

    static void drawQuad(const glm::mat4& transform, const glm::vec4& color,
                         const std::string& textureName = DEFAULT_TEX) {
        const std::array<glm::vec4, 4> vertexCoords = {{
            {-0.5f, -0.5f, 0.0f, 1.0f},
            {0.5f, -0.5f, 0.0f, 1.0f},
            {0.5f, 0.5f, 0.0f, 1.0f},
            {-0.5f, 0.5f, 0.0f, 1.0f},
        }};

        if (sceneData->quadIndexCount >= sceneData->MAX_IND) {
            next_batch();
        }

        auto textureStatus = textureLibrary.isTextureOrSubtexture(textureName);

        std::shared_ptr<Texture> texture;
        std::shared_ptr<Subtexture> subtexture;

        if (textureStatus == -1 || textureStatus == 0) {
            texture = textureLibrary.get(textureName);
        } else {
            subtexture = textureLibrary.getSubtexture(textureName);
            // only do this -> when valid, to avoid segfault
            // we check for validity later so this is fine
            if (subtexture) texture = subtexture->texture;
        }

        // Load the corresponding Texture into the texture slots
        float textureIndex = 0.f;
        if (texture                        // tex is valid (ie not nullptr)
            && textureName != DEFAULT_TEX  // default tex is always loaded
            && textureStatus != -1  // set to default tex so already loaded
        ) {
            for (int i = 1; i < sceneData->nextTexSlot; i++) {
                if (*(sceneData->textureSlots[i]) == *texture) {
                    textureIndex = (float)i;
                    break;
                }
            }
            if (textureIndex == 0) {
                if (sceneData->nextTexSlot >= MAX_TEX) {
                    next_batch();
                }
                textureIndex = sceneData->nextTexSlot;
                sceneData->textureSlots[textureIndex] =
                    dynamic_pointer_cast<Texture2D>(texture);
                sceneData->nextTexSlot++;

                stats.textureCount++;
            }
        } else {
            texture = textureLibrary.get(DEFAULT_TEX);
            // if we fall into this case,
            // either textureName didnt exist at all
            // or textureName was a texture and is invalid
            // or textureName was an invalid subtexture or texture is
            textureStatus = -1;
        }
        // else use 0 which is white texture

        for (size_t i = 0; i < 4; i++) {
            sceneData->qvbufferptr->position = transform * vertexCoords[i];
            sceneData->qvbufferptr->color = color;
            sceneData->qvbufferptr->texcoord =
                textureStatus != 1 ? texture->textureCoords[i]
                                   : subtexture->textureCoords[i];
            sceneData->qvbufferptr->texindex = textureIndex;
            sceneData->qvbufferptr++;
        }
        sceneData->quadIndexCount += 6;

        stats.quadCount++;
    }

    // for (int i = 0; i < 360; i += 10) {
    // Renderer::drawLine(glm::vec3{0.f, 0.f, 0.f},
    // glm::vec2{
    // 10.f * sin(glm::radians(1.f * i)),
    // 10.f * cos(glm::radians(1.f * i)),
    // },
    // glm::vec4{1, 1, 1, 1});
    // }
    static void drawLine(const glm::vec3& start, const glm::vec3& end,
                         const glm::vec4& color) {
        sceneData->lvbufferptr->position = start;
        sceneData->lvbufferptr->color = color;
        sceneData->lvbufferptr++;

        sceneData->lvbufferptr->position = end;
        sceneData->lvbufferptr->color = color;
        sceneData->lvbufferptr++;

        sceneData->lineVertexCount += 2;
    }

    static void drawPolygon(const std::vector<glm::vec2> points,
                            const glm::vec4& color) {
        if (points.empty()) return;

        for (int i = 0; i < (int)points.size() + 2; i++) {
            sceneData->pvbufferptr->position =
                glm::vec3{points[i % points.size()], 0.f};
            sceneData->pvbufferptr->color = color;

            sceneData->pvbufferptr++;
            sceneData->polyVertexCount += 1;
        }

        next_batch();
    }

    static void setLineThickness(float thickness) { glLineWidth(thickness); };

    ////// ////// ////// ////// ////// ////// ////// //////
    //      the draw calls below here, just call one of the ones above
    ////// ////// ////// ////// ////// ////// ////// //////

    static void drawLine(const glm::vec2& start, const glm::vec2& end,
                         const glm::vec4& color) {
        Renderer::drawLine(glm::vec3{start, 0.f}, glm::vec3{end, 0.f}, color);
    }

    static void drawQuad(const glm::vec2& position, const glm::vec2& size,
                         const glm::vec4& color,
                         const std::string& textureName = DEFAULT_TEX) {
        Renderer::drawQuad(glm::vec3{position.x, position.y, 0.f}, size, color,
                           textureName);
    }

    static void drawQuad(const glm::vec3& position, const glm::vec2& size,
                         const glm::vec4& color,
                         const std::string& textureName = DEFAULT_TEX) {
        auto transform = glm::translate(imat, position) *
                         glm::scale(imat, {size.x, size.y, 1.0f});
        Renderer::drawQuad(transform, color, textureName);
    }

    static void drawQuadRotated(const glm::vec2& position,
                                const glm::vec2& size, float angleInRad,
                                const glm::vec4& color,
                                const std::string& textureName = DEFAULT_TEX) {
        Renderer::drawQuadRotated({position.x, position.y, 0.f}, size,
                                  angleInRad, color, textureName);
    }

    static void drawQuadRotated(const glm::vec3& position,
                                const glm::vec2& size, float angleInRad,
                                const glm::vec4& color,
                                const std::string& textureName = DEFAULT_TEX) {
        prof p(__PROFILE_FUNC__);

        auto transform = glm::translate(imat, position) *
                         glm::rotate(imat, angleInRad, {0.0f, 0.0f, 1.f}) *
                         // TODO We need this in order for the rotation to
                         // happen kinda close to the center of the object
                         glm::translate(imat, {size.x / 4, size.y / 4, 0.f}) *
                         glm::scale(imat, {size.x, size.y, 1.0f});

        drawQuad(transform, color, textureName);
    }
};
