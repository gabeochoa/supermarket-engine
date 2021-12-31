

#include "../engine/app.h"
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
#include "tests.h"
#include "uitest.h"

//
/*

   Game Ideas

Allow player to label aisles which make it easier for people to find the item
the want

Customers leaving items around
 + Another stock job to put items back
Stealing items

Weather and day night system
 + Different desires based in


  Engine Ideas

- support for defer/runlater
    for example:
        {
        auto X = new int[10];
        defer(free x)
        ... rest of function
        } // free happens here

- LRU cache for temporary textures
- base64 encode, is this already built in c++ ?
- add support for editing values on the fly
    some way to register values with global storage
    hook up a UI dropdown to edit it
    support new debug layer for holding the ui dropdown
    add support for adding a struct (is there some kind of Reflection in cpp?)
        automatically discover AND create a UI to edit the values
- add charts to UI lib
- build Task and Task managing into the engine?
    other threads for really simple stuff only?
- support for playing audio
- ai behavior tree ?
    i prefer a job based system but it doesnt support interruptions right now
- decompression library
    create / read bins so textures/fonts/etc arent leaked
    probably can do some kind of encryption on the file too
- radar ui would look cool
    https://github.com/s9w/oof/blob/master/demos/radar_demo.cpp
- async coroutines
    http://libdill.org/structured-concurrency.html
- Make UI state reactive
    https://github.com/snakster/cpp.react
- networking
    socket.io?
- support for OTF font formats
    https://github.com/caryll/otfcc




*/

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

    // test code underneath game so it never shows
    Layer* uitest = new UITestLayer();
    App::get().pushLayer(uitest);

    //
    Layer* super = new SuperLayer();
    App::get().pushLayer(super);
    Layer* game_ui = new GameUILayer();
    App::get().pushLayer(game_ui);

    // TODO integrate into gameUI layer
    Layer* jobLayer = new JobLayer();
    App::get().pushLayer(jobLayer);

    Layer* menu = new MenuLayer();
    App::get().pushLayer(menu);

    //
    Layer* profile = new ProfileLayer();
    App::get().pushLayer(profile);
    Layer* entityDebug = new EntityDebugLayer();
    App::get().pushLayer(entityDebug);

    // Menu::get().state = Menu::State::Game;
    Menu::get().state = Menu::State::Root;
    // Menu::get().state = Menu::State::UITest;

    App::get().run();

    return 0;
}
