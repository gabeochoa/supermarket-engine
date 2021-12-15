
#pragma once

#include <cassert>
#include <functional>
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
constexpr auto LOG_LEVEL = LogLevel::INFO;

inline const char* level_to_string(int level) {
    switch (level) {
        default:
        case LogLevel::ALL:
            return "";
        case LogLevel::TRACE:
            return "Trace";
        case LogLevel::INFO:
            return "Info";
        case LogLevel::WARN:
            return "Warn";
        case LogLevel::ERROR:
            return "Error";
    }
}

inline void vlog(int level, const char* file, int line, fmt::string_view format,
                 fmt::format_args args) {
    if (level < LOG_LEVEL) return;
    fmt::print("{}: {}: {}: ", file, line, level_to_string(level));
    fmt::vprint(format, args);
    fmt::print("\n");
}

template <typename... Args>
inline void log_me(int level, const char* file, int line, const char* format,
                   Args&&... args) {
    vlog(level, file, line, format,
         fmt::make_args_checked<Args...>(format, args...));
}

template <>
inline void log_me(int level, const char* file, int line, const char* format,
                   const char*&& args) {
    vlog(level, file, line, format,
         fmt::make_args_checked<const char*>(format, args));
}

#define log_trace(...) log_me(LogLevel::TRACE, __FILE__, __LINE__, __VA_ARGS__)

#define log_info(...) log_me(LogLevel::INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) log_me(LogLevel::WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...)                                        \
    log_me(LogLevel::ERROR, __FILE__, __LINE__, __VA_ARGS__); \
    assert(false)

#define M_ASSERT(x, ...)                                    \
    {                                                       \
        if (!(x)) {                                         \
            log_error("Assertion failed: {}", __VA_ARGS__); \
        }                                                   \
    }
