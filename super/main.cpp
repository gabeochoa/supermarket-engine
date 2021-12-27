

#include "../engine/app.h"
#include "../engine/file.h"
#include "../engine/input.h"
#include "../engine/pch.hpp"
#include "custom_fmt.h"
#include "entities.h"
#include "job.h"
#include "menu.h"
#include "util.h"

// defines cameras
#include "global.h"

// Requires access to the camera and entitites
#include "debug_layers.h"
#include "menulayer.h"
#include "superlayer.h"
#include "tests.h"
#include "uitest.h"

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    // on start tests
    {
        theta_test();
        point_collision_test();

        {  // make sure linear interp always goes up
            float c = 0.f;
            auto a = LinearInterp(0.f, 1.f, 100);
            for (int i = 0; i < 100; i++) {
                auto b = a.next();
                M_ASSERT(c - b < 0.1, "should linearly interpolate");
                c += 0.01;
            }
        }
        // test_file_functionality();
    }

    app.reset(App::create({
        .width = WIN_W,
        .height = WIN_H,
        .title = "SuperMarket",
        .clearEnabled = true,
        .escClosesWindow = false,
    }));

    Layer* super = new SuperLayer();
    App::get().pushLayer(super);

    Layer* menu = new MenuLayer();
    App::get().pushLayer(menu);

    Layer* uitest = new UITestLayer();
    App::get().pushLayer(uitest);

    Layer* profile = new ProfileLayer();
    App::get().pushLayer(profile);

    Layer* entityDebug = new EntityDebugLayer();
    App::get().pushLayer(entityDebug);

    Layer* jobLayer = new JobLayer();
    App::get().pushLayer(jobLayer);

    // Menu::get().state = Menu::State::Root;
    Menu::get().state = Menu::State::UITest;

    App::get().run();

    return 0;
}
