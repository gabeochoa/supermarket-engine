

#include "../engine/app.h"
#include "../engine/input.h"
#include "../engine/pch.hpp"
#include "custom_fmt.h"
#include "entities.h"
#include "job.h"
#include "vecutil.h"

// TODO at some point we should have some way to send this to app
constexpr int WIN_W = 1920;
constexpr int WIN_H = 1080;
constexpr float WIN_RATIO = (WIN_W * 1.f) / WIN_H;

constexpr bool IS_DEBUG = true;

struct SuperLayer : public Layer {
    std::vector<std::shared_ptr<Entity>> entities;

    OrthoCameraController cameraController;

    SuperLayer() : Layer("Supermarket"), cameraController(WIN_RATIO, true) {
        auto billy = std::make_shared<Billboard>(
            glm::vec2{0.f, 0.f}, glm::vec2{1.f, 1.f}, 45.f,
            glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}, "face");
        entities.push_back(billy);

        billy = std::make_shared<Billboard>(glm::vec2{0.5f, 1.f},
                                            glm::vec2{0.5f, 0.5f}, 0.f,
                                            glm::vec4{0.8f, 0.3f, 0.0f, 1.0f});
        entities.push_back(billy);

        billy = std::make_shared<Billboard>(glm::vec2{-0.5f, -1.f},
                                            glm::vec2{1.1f, 1.1f}, 0.f,
                                            glm::vec4{0.2f, 0.7f, 0.0f, 1.0f});
        entities.push_back(billy);

        Job j = {.type = JobType::None, .seconds = 150};
        JobQueue::addJob(JobType::None, std::make_shared<Job>(j));

        for (int i = 0; i < 10; i++) {
            auto emp = Employee();
            // TODO eventually get a texture
            // and fix colored textures
            emp.textureName = "__INVALID__";
            emp.color = gen_rand_vec4(0.3f, 1.0f);
            emp.color.w = 1.f;
            entities.push_back(std::make_shared<Employee>(emp));
        }
    }

    virtual ~SuperLayer() {}
    virtual void onAttach() override {}
    virtual void onDetach() override {}

    virtual void onUpdate(Time dt) override {
        log_trace(fmt::format("{:.2}s ({:.2} ms) ", dt.s(), dt.ms()));
        prof(__PROFILE_FUNC__);

        cameraController.onUpdate(dt);
        for (auto& entity : entities) {
            entity->onUpdate(dt);
        }

        Renderer::clear(/* color */ {0.1f, 0.1f, 0.1f, 1.0f});
        Renderer::begin(cameraController.camera);

        for (auto& entity : entities) {
            entity->render();
        }

        Renderer::end();

        if (JobQueue::numOfJobsWithType(JobType::IdleWalk) < 5) {
            JobQueue::addJob(
                JobType::IdleWalk,
                std::make_shared<Job>(
                    Job({.type = JobType::IdleWalk,
                         .startPosition = glm::circularRand<float>(5.f),
                         .endPosition = glm::circularRand<float>(5.f)})));
        }
        // Cleanup all completed jobs
        JobQueue::cleanup();
    }

    virtual void onEvent(Event& event) override {
        log_trace(event.toString());
        cameraController.onEvent(event);
    }
};

struct ProfileLayer : public Layer {
    bool showFilenames;
    ProfileLayer() : Layer("Profiling"), showFilenames(false) {
        isMinimized = !IS_DEBUG;
    }

    virtual ~ProfileLayer() {}
    virtual void onAttach() override {}
    virtual void onDetach() override {}

    GLTtext* drawText(const std::string& content, int x, int y, float scale) {
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
        int y = 10;
        float scale = 1.f;
        std::vector<GLTtext*> texts;
        gltBeginDraw();

        std::vector<SamplePair> pairs;
        for (const auto& x : _acc) {
            pairs.push_back(x);
        }

        sort(pairs.begin(), pairs.end(),
             [](const SamplePair& a, const SamplePair& b) {
                 return a.second.average() > b.second.average();
             });

        for (const auto& x : pairs) {
            auto name = x.first;
            auto stats = x.second;
            auto filename = showFilenames ? stats.filename : "";
            std::string t = fmt::format("{}{}: avg: {:.2f}ns", filename, name,
                                        stats.average());
            texts.push_back(drawText(t, 10, y, scale));
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
            JobType type = (JobType)x.first;
            auto job_list = x.second;
            int num_assigned =
                std::count_if(job_list.begin(), job_list.end(),
                              [](const std::shared_ptr<Job>& job) {
                                  return job->isAssigned;
                              });
            std::string t =
                fmt::format("{}: {} ({} assigned)", jobTypeToString(type),
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

        return false;
    }

    virtual void onEvent(Event& event) override {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<KeyPressedEvent>(std::bind(
            &ProfileLayer::onKeyPressed, this, std::placeholders::_1));
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    App::get();

    Layer* profile = new ProfileLayer();
    App::get().pushLayer(profile);

    Layer* super = new SuperLayer();
    App::get().pushLayer(super);

    App::get().run();
    return 0;
}
