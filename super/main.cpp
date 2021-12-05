


#include "../engine/pch.hpp"
#include "../engine/app.h"

struct Super{};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    App::get().run();
    return 0;
}
