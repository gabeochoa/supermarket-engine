
#pragma once

#include <cassert>
#include <iostream>
#include <string>

enum LogLevel : int {
    ALL = 0,
    TRACE,
    INFO,
    WARN,
    ERROR,
};

// TODO lets move this to an env var or something
// we shouldnt have to recompile to change the level
constexpr auto LOG_LEVEL = LogLevel::TRACE;
#define M_SHOULD_LOG(x)             \
    {                               \
        if (x <= LOG_LEVEL) return; \
    }

// TODO replace having to do fmt::format() for logs, check the api docs theres
// definitely a way to do it

inline void log(const std::string &line) { std::cout << line << std::endl; }
inline void log_trace(const std::string &s) {
    M_SHOULD_LOG(LogLevel::TRACE);
    log(fmt::format("TRACE: {}", s));
}
inline void log_info(const std::string &s) {
    M_SHOULD_LOG(LogLevel::INFO);
    log(fmt::format("INFO: {}", s));
}
inline void log_warn(const std::string &s) {
    M_SHOULD_LOG(LogLevel::WARN);
    log(fmt::format("WARN: {}", s));
}
inline void log_error(const std::string &s) {
    M_SHOULD_LOG(LogLevel::ERROR);
    log(fmt::format("ERROR: {}", s));
    assert(false);
}

#define M_ASSERT(x, ...)                                                 \
    {                                                                    \
        if (!(x)) {                                                      \
            log_error(fmt::format("Assertion failed: {}", __VA_ARGS__)); \
        }                                                                \
    }
