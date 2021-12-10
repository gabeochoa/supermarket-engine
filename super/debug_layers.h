
#pragma once

#include "../engine/pch.hpp"

struct ProfileLayer : public Layer {
    bool showFilenames;
    float seconds;

    ProfileLayer() : Layer("Profiling"), showFilenames(false) {
        isMinimized = true;  //! IS_DEBUG;
        // Reset all profiling
        _acc.clear();
        seconds = 0.f;
    }

    virtual ~ProfileLayer() {}
    virtual void onAttach() override {}
    virtual void onDetach() override {}

    GLTtext* drawText(const std::string& content, int x, int y, float scale) {
        GLTtext* text = gltCreateText();
        gltSetText(text, content.c_str());
        gltDrawText2D(text, x, y, scale);
        return text;
    }

    virtual void onUpdate(Time dt) override {
        prof(__PROFILE_FUNC__);
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

        std::vector<SamplePair> pairs;
        pairs.insert(pairs.end(), _acc.begin(), _acc.end());
        sort(pairs.begin(), pairs.end(),
             [](const SamplePair& a, const SamplePair& b) {
                 return a.second.average() > b.second.average();
             });

        for (const auto& x : pairs) {
            auto stats = x.second;
            texts.push_back(
                drawText(fmt::format("{}{}: avg: {:.2f}ns",
                                     showFilenames ? stats.filename : "",
                                     x.first, stats.average()),
                         10, y, scale));
            y += 30;
        }

        texts.push_back(
            drawText(fmt::format("Press delete to toggle filenames {}",
                                 showFilenames ? "off" : "on"),
                     0, y, scale));
        y += 30;

        // extra space
        y += 60;

        // Job queue
        texts.push_back(drawText("Job Queue", 0, y, scale));
        y += 30;

        for (const auto& x : jobs) {
            auto job_list = x.second;
            int num_assigned = 0;
            for (auto it = job_list.begin(); it != job_list.end(); it++) {
                if ((*it)->isAssigned) num_assigned++;
            }
            std::string t = fmt::format("{}: {} ({} assigned)",
                                        jobTypeToString((JobType)x.first),
                                        job_list.size(), num_assigned);
            texts.push_back(drawText(t, 10, y, scale));
            y += 30;
        }
        // end job queue

        gltEndDraw();
        for (auto text : texts) gltDeleteText(text);
        gltTerminate();
    }

    bool onKeyPressed(KeyPressedEvent event) {
        if (event.keycode == Key::mapping["Open Profiler"]) {
            isMinimized = !isMinimized;
        }

        if (event.keycode == Key::mapping["Profiler Hide Filenames"]) {
            showFilenames = !showFilenames;
        }
        if (event.keycode == Key::mapping["Profiler Clear Stats"]) {
            _acc.clear();
        }
        // log_info(std::to_string(event.keycode));
        return false;
    }

    virtual void onEvent(Event& event) override {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<KeyPressedEvent>(std::bind(
            &ProfileLayer::onKeyPressed, this, std::placeholders::_1));
    }
};
struct EntityDebugLayer : public Layer {
    EntityDebugLayer() : Layer("EntityDebug") { isMinimized = !IS_DEBUG; }

    virtual ~EntityDebugLayer() {}
    virtual void onAttach() override {}
    virtual void onDetach() override {}

    GLTtext* drawText(const std::string& content, float x, float y,
                      float scale) {
        GLTtext* text = gltCreateText();
        gltSetText(text, content.c_str());
        gltColor(1.0f, 1.0f, 1.0f, 1.0f);
        gltDrawText2D(text, x, y, scale);
        return text;
    }

    virtual void onUpdate(Time dt) override {
        (void)dt;
        if (isMinimized) {
            return;
        }

        prof(__PROFILE_FUNC__);

        gltInit();
        float scale = 0.003f;
        std::vector<GLTtext*> texts;
        gltColor(1.0f, 1.0f, 1.0f, 1.0f);
        gltBeginDraw();

        for (auto& e : entities) {
            auto s = fmt::format("{}", *e);
            GLTtext* text = gltCreateText();
            gltSetText(text, s.c_str());

            // V = C^-1
            auto V = cameraController->camera.view;
            auto P = cameraController->camera.projection;
            auto pos = glm::vec3{
                e->position.x,
                e->position.y + gltGetTextHeight(text, scale),  //
                0.f                                             //
            };
            // M = T * R * S
            auto M =  // model matrix
                glm::translate(glm::mat4(1.f), pos) *
                glm::scale(glm::mat4(1.0f), {scale, -scale, 1.f});

            auto mvp = P * V * M;

            gltDrawText(text, glm::value_ptr(mvp));
            texts.push_back(text);
        }

        gltEndDraw();
        for (auto text : texts) gltDeleteText(text);
        gltTerminate();
    }

    bool onKeyPressed(KeyPressedEvent event) {
        if (event.keycode == Key::mapping["Show Entity Overlay"]) {
            isMinimized = !isMinimized;
        }
        return false;
    }

    virtual void onEvent(Event& event) override {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<KeyPressedEvent>(std::bind(
            &EntityDebugLayer::onKeyPressed, this, std::placeholders::_1));
    }
};

