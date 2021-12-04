
#pragma once

#include <cassert>
#include <iostream>

#include "event.h"
#include "pch.hpp"

void log(const std::string &line) { std::cout << line << std::endl; }
void log(Event &e) { std::cout << e << std::endl; }
void log_error(const std::string &s) {
    log(s);
    assert(false);
}

#define M_ASSERT(x, ...)                                                 \
    {                                                                    \
        if (!(x)) {                                                      \
            log_error(fmt::format("Assertion failed: {}", __VA_ARGS__)); \
        }                                                                \
    }
