

#include "../engine/app.h"
#include "../engine/input.h"
#include "../engine/pch.hpp"

struct SuperLayer : public Layer {
    OrthoCamera camera;
    float camSpeed;
    float rotSpeed;

    std::shared_ptr<Shader> shader;
    std::shared_ptr<Shader> flatShader;
    std::shared_ptr<Shader> textureShader;

    std::shared_ptr<Texture> screenTexture;
    std::shared_ptr<Texture> faceTexture;

    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<VertexArray> squareVA;
    std::shared_ptr<VertexArray> squareVA2;

    SuperLayer()
        : Layer("Supermarket"),
          camera(-1.6f, 1.6f, -0.9f, 0.9f),
          camSpeed(5.f),
          rotSpeed(180.f) {
        camera.position.x += 0.5f;
        camera.position.y += 0.5f;

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

        flatShader.reset(new Shader(flat_vert_s, flat_frag_s));

        textureShader.reset(new Shader(tex_vert_s, tex_frag_s));

        faceTexture.reset(new Texture2D("./resources/face.png"));
        screenTexture.reset(new Texture2D("./resources/screen.png", 1));
    }

    virtual ~SuperLayer() {}
    virtual void onAttach() override {}
    virtual void onDetach() override {}

    void handleLiveInput(Time dt) {
        if (Input::isKeyPressed(Key::mapping["Left"])) {
            camera.position.x -= camSpeed * dt;
        }
        if (Input::isKeyPressed(Key::mapping["Right"])) {
            camera.position.x += camSpeed * dt;
        }
        if (Input::isKeyPressed(Key::mapping["Down"])) {
            camera.position.y -= camSpeed * dt;
        }
        if (Input::isKeyPressed(Key::mapping["Up"])) {
            camera.position.y += camSpeed * dt;
        }
        if (Input::isKeyPressed(Key::mapping["Rotate Clockwise"])) {
            camera.rotation += rotSpeed * dt;
        }
        if (Input::isKeyPressed(Key::mapping["Rotate Counterclockwise"])) {
            camera.rotation -= rotSpeed * dt;
        }
        camera.updateViewMat();
    }

    virtual void onUpdate(Time dt) override {
        // flatShader->bind();
        // flatShader->uploadUniformFloat4("u_color",
        // glm::vec4(0.8f, 0.2f, 0.3f, 1.0f));
        log_trace(fmt::format("{:.2}s ({:.2} ms) ", dt.s(), dt.ms()));

        handleLiveInput(dt);

        Renderer::clear({0.1f, 0.1f, 0.1f, 1.0f});
        Renderer::begin(camera);

        textureShader->bind();
        screenTexture->bind(1);
        textureShader->uploadUniformInt("u_texture", 1);
        Renderer::submit(squareVA2, textureShader);

        textureShader->bind();
        faceTexture->bind(0);
        textureShader->uploadUniformInt("u_texture", 0);
        Renderer::submit(squareVA, textureShader);

        Renderer::end();
    }

    bool onKeyPressed(KeyPressedEvent& event) {
        (void)event;
        return false;
    }

    virtual void onEvent(Event& event) override {
        log_trace(event.toString());
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<KeyPressedEvent>(
            std::bind(&SuperLayer::onKeyPressed, this, std::placeholders::_1));
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
