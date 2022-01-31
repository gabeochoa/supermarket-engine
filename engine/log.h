
#pragma once

#include <cassert>
#include <functional>
#include <iostream>
#include <string>

#include "external_include.h"

namespace superlog {
enum LogLevel : int {
    SUPER_LOG_ALL = 0,
    SUPER_LOG_TRACE,
    SUPER_LOG_INFO,
    SUPER_LOG_WARN,
    SUPER_LOG_ERROR,
};

// TODO lets move this to an env var or something
// we shouldnt have to recompile to change the level
static auto LOG_LEVEL = LogLevel::SUPER_LOG_INFO;

inline const char* level_to_string(int level) {
    switch (level) {
        default:
        case LogLevel::SUPER_LOG_ALL:
            return "";
        case LogLevel::SUPER_LOG_TRACE:
            return "Trace";
        case LogLevel::SUPER_LOG_INFO:
            return "Info";
        case LogLevel::SUPER_LOG_WARN:
            return "Warn";
        case LogLevel::SUPER_LOG_ERROR:
            return "Error";
    }
}

}  // namespace superlog

inline void vlog(int level, const char* file, int line, fmt::string_view format,
                 fmt::format_args args) {
    if (level < superlog::LOG_LEVEL) return;
    fmt::print("{}: {}: {}: ", file, line, superlog::level_to_string(level));
    fmt::vprint(format, args);
    fmt::print("\n");
}

inline void vlog(int level, const char* file, int line,
                 fmt::wstring_view format, fmt::wformat_args args) {
    if (level < superlog::LOG_LEVEL) return;
    fmt::print("{}: {}: {}: ", file, line, superlog::level_to_string(level));
    fmt::vprint(format, args);
    fmt::print("\n");
}

template <typename... Args>
inline void log_me(int level, const char* file, int line, const char* format,
                   Args&&... args) {
    vlog(level, file, line, format,
         fmt::make_args_checked<Args...>(format, args...));
}

template <typename... Args>
inline void log_me(int level, const char* file, int line, const wchar_t* format,
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

template <>
inline void log_me(int level, const char* file, int line, const wchar_t* format,
                   const wchar_t*&& args) {
    vlog(level, file, line, format,
         fmt::make_args_checked<const wchar_t*>(format, args));
}

#define log_trace(...) \
    log_me(superlog::LogLevel::SUPER_LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)

#define log_info(...) \
    log_me(superlog::LogLevel::SUPER_LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) \
    log_me(superlog::LogLevel::SUPER_LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...)                                              \
    log_me(superlog::LogLevel::SUPER_LOG_ERROR, __FILE__, __LINE__, \
           __VA_ARGS__);                                            \
    assert(false)

#define M_ASSERT(x, ...)                                    \
    {                                                       \
        if (!(x)) {                                         \
            log_error("Assertion failed: {}", __VA_ARGS__); \
        }                                                   \
    }
