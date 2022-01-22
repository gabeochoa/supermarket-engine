
#pragma once

#include "app.h"
#include "layer.h"
#include "pch.hpp"
#include "time.h"

struct FPSLayer : public Layer {
    FPSLayer() : Layer("FPS") {
        isMinimized = false;
        GLOBALS.set<bool>("hide_fps", &isMinimized);
    }
    virtual ~FPSLayer() {}
    virtual void onAttach() override {}
    virtual void onDetach() override {}

    inline GLTtext* drawText(const std::string& content, int x, int y,
                             float scale) {
        GLTtext* text = gltCreateText();
        gltSetText(text, content.c_str());
        gltDrawText2D(text, x, y, scale);
        return text;
    }

    virtual void onUpdate(Time dt) override {
        prof give_me_a_name(__PROFILE_FUNC__);
        (void)dt;

        if (isMinimized) {
            return;
        }

        int y = 10;
        float scale = 1.f;
        gltInit();
        gltBeginDraw();
        gltColor(1.0f, 1.0f, 1.0f, 1.0f);
        std::vector<GLTtext*> texts;

        auto avgRenderTime =
            Renderer::stats.totalFrameTime / Renderer::stats.renderTimes.size();
        texts.push_back(
            drawText(fmt::format("Avg render time {:.4f} ms ({:.2f} fps)",
                                 avgRenderTime, 1.f / avgRenderTime),
                     App::getSettings().width - 320, y, scale));
        y += 30;

        gltEndDraw();
        for (auto text : texts) gltDeleteText(text);
        gltTerminate();
    }

    virtual void onEvent(Event& event) override {
        EventDispatcher dispatcher(event);
        (void)event;
        (void)dispatcher;
    }
};

