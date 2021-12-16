
#pragma once

#include "../engine/pch.hpp"

struct Menu;
static std::shared_ptr<Menu> menu;

struct Menu {
    enum State {
        Root = 0,
        Game,
        Paused,
    };

    State state;
    inline static Menu* create() { return new Menu(); }
    inline static Menu& get() {
        if (!menu) menu.reset(Menu::create());
        return *menu;
    }
};

inline const char* stateToString(Menu::State state) {
    switch (state) {
        case Menu::State::Root:
            return "Root";
        case Menu::State::Game:
            return "Game";
        case Menu::State::Paused:
            return "Paused";
        default:
            log_warn("Missing menu state in stateToString ");
            return "MenuState no stateToString";
    }
}

