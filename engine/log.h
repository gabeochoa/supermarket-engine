
#pragma once

#include <cassert>
#include <iostream>
#include <string>

inline void log(const std::string &line) { std::cout << line << std::endl; }
inline void log_warn(const std::string &s) {
    log(fmt::format("WARN: {}", s));
}
inline void log_error(const std::string &s) {
    log(fmt::format("ERROR: {}", s));
    assert(false);
}

#define M_ASSERT(x, ...)                                                 \
    {                                                                    \
        if (!(x)) {                                                      \
            log_error(fmt::format("Assertion failed: {}", __VA_ARGS__)); \
        }                                                                \
    }
