

#include "../vendor/backward.hpp"
//

#include "../engine/app.h"
#include "../engine/edit.h"
#include "../engine/file.h"
#include "../engine/input.h"
#include "../engine/pch.hpp"
#include "../engine/strutil.h"
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
#include "terminal_layer.h"
#include "tests.h"
#include "uitest.h"

void add_globals() {
    //
    GLOBALS.set("menu_state", menu.get());
    EDITOR_COMMANDS.registerCommand("set_menu_state", SetValueCommand<int>(),
                                    "Change Menu:: state");
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    backward::SignalHandling sh;

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

        test_global_commands();
    }

    add_globals();

    app.reset(App::create({
        .width = WIN_W,
        .height = WIN_H,
        .title = "SuperMarket",
        .clearEnabled = true,
        .escClosesWindow = false,
    }));

    Layer* terminal = new TerminalLayer();
    App::get().pushLayer(terminal);

    Layer* fps = new FPSLayer();
    App::get().pushLayer(fps);

    Layer* profile = new ProfileLayer();
    App::get().pushLayer(profile);
    Layer* entityDebug = new EntityDebugLayer();
    App::get().pushLayer(entityDebug);

    Layer* menu = new MenuLayer();
    App::get().pushLayer(menu);

    Layer* game_ui = new GameUILayer();
    App::get().pushLayer(game_ui);

    // TODO integrate into gameUI layer
    // Layer* jobLayer = new JobLayer();
    // App::get().pushLayer(jobLayer);
    //
    Layer* super = new SuperLayer();
    App::get().pushLayer(super);

    // test code underneath game so it never shows
    Layer* uitest = new UITestLayer();
    App::get().pushLayer(uitest);

    Menu::get().state = Menu::State::Game;
    // Menu::get().state = Menu::State::Root;
    // Menu::get().state = Menu::State::UITest;

    App::get().run();

    return 0;
}
