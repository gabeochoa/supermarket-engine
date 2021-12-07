

#include "../engine/app.h"
#include "../engine/input.h"
#include "../engine/pch.hpp"

struct SuperLayer : public Layer {
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<VertexArray> squareVA;
    std::shared_ptr<VertexArray> squareVA2;

    ShaderLibrary shaderLibrary;
    TextureLibrary textureLibrary;

    OrthoCameraController cameraController;

    SuperLayer()
        : Layer("Supermarket"), cameraController(1920.f / 1080.f, true) {
        {
            std::shared_ptr<VertexBuffer> squareVB;
            std::shared_ptr<IndexBuffer> squareIB;
            squareVA.reset(VertexArray::create());

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
            squareVB.reset(
                VertexBuffer::create(squareVerts, sizeof(squareVerts)));
            squareVB->setLayout(BufferLayout{
                {"i_pos", BufferType::Float3},
                {"i_texcoord", BufferType::Float2},
            });
            squareVA->addVertexBuffer(squareVB);

            unsigned int squareIs[] = {0, 1, 2, 0, 2, 3};
            squareIB.reset(IndexBuffer::create(squareIs, 6));
            squareVA->setIndexBuffer(squareIB);
        }

        {
            std::shared_ptr<VertexBuffer> squareVB;
            std::shared_ptr<IndexBuffer> squareIB;
            squareVA2.reset(VertexArray::create());
            float squareVerts[5 * 4] = {
                0.f, 0.f, 0.f, 0.0f, 0.0f,  //
                1.f, 0.f, 0.f, 1.0f, 0.0f,  //
                1.f, 1.f, 0.f, 1.0f, 1.0f,  //
                0.f, 1.f, 0.f, 0.0f, 1.0f,  //
            };
            squareVB.reset(
                VertexBuffer::create(squareVerts, sizeof(squareVerts)));
            squareVB->setLayout(BufferLayout{
                {"i_pos", BufferType::Float3},
                {"i_texcoord", BufferType::Float2},
            });
            squareVA2->addVertexBuffer(squareVB);

            unsigned int squareIs[] = {0, 1, 2, 0, 2, 3};
            squareIB.reset(IndexBuffer::create(squareIs, 6));
            squareVA2->setIndexBuffer(squareIB);
        }

        shaderLibrary.load("./engine/shaders/flat.glsl");
        shaderLibrary.load("./engine/shaders/texture.glsl");

        textureLibrary.load("./resources/face.png");
        textureLibrary.load("./resources/screen.png", 1);
    }

    virtual ~SuperLayer() {}
    virtual void onAttach() override {}
    virtual void onDetach() override {}

    virtual void onUpdate(Time dt) override {
        cameraController.onUpdate(dt);

        log_trace(fmt::format("{:.2}s ({:.2} ms) ", dt.s(), dt.ms()));

        Renderer::clear({0.1f, 0.1f, 0.1f, 1.0f});
        Renderer::begin(cameraController.camera);

        auto textureShader = shaderLibrary.get("texture");
        textureShader->bind();
        textureShader->uploadUniformInt("u_texture", 1);
        textureLibrary.get("screen")->bind(1);
        Renderer::submit(squareVA2, textureShader);

        textureShader = shaderLibrary.get("texture");
        textureShader->bind();
        textureShader->uploadUniformInt("u_texture", 0);
        textureLibrary.get("face")->bind(0);
        Renderer::submit(squareVA, textureShader);

        Renderer::end();
    }

    virtual void onEvent(Event& event) override {
        log_trace(event.toString());
        cameraController.onEvent(event);
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    App::get();

    SuperLayer* super = new SuperLayer();
    App::get().pushLayer(super);

    App::get().run();
    return 0;
}
