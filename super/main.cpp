

#include "../engine/app.h"
#include "../engine/input.h"
#include "../engine/pch.hpp"
#include "custom_fmt.h"
#include "entities.h"
#include "job.h"
#include "util.h"

// TODO at some point we should have some way to send this to app
constexpr int WIN_W = 1920;
constexpr int WIN_H = 1080;
constexpr float WIN_RATIO = (WIN_W * 1.f) / WIN_H;
constexpr bool IS_DEBUG = true;

static std::shared_ptr<OrthoCameraController> cameraController;

// Requires access to the camera and entitites
#include "debug_layers.h"
#include "superlayer.h"
#include "tests.h"

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    // on start tests
    {
        theta_test();
        point_collision_test();
    }

    app.reset(App::create({
        .width = 1920,
        .height = 1080,
        .title = "SuperMarket",
        .clearEnabled = true,
    }));

    Layer* super = new SuperLayer();
    App::get().pushLayer(super);

    Layer* profile = new ProfileLayer();
    App::get().pushLayer(profile);

    Layer* entityDebug = new EntityDebugLayer();
    App::get().pushLayer(entityDebug);

    Layer* jobLayer = new JobLayer();
    App::get().pushLayer(jobLayer);

    App::get().run();

    return 0;
}
