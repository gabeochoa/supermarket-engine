
#pragma once

#include <chrono>
#include <deque>
#include <map>
#include <string>

#include "external_include.h"

struct Time {
    const char* name;

    float last;
    float delta;

    void start() { last = glfwGetTime(); }

    void end() {
        float now = (float)glfwGetTime();
        delta = now - last;
        last = now;
    }

    operator float() { return s(); }
    float ms() const { return delta * 1000.f; }
    float s() const { return delta; }
};

struct Samples {
    std::string filename;
    int size = 100;
    float samples[100];
    int index;
    int numItems;
    float sum, avg, min, max;
    Samples()
        : filename("unknown"),
          index(0),
          numItems(0),
          sum(0),
          avg(0),
          min(0),
          max(0) {}

    void setFilename(const std::string& fn) { filename = fn; }

    float& operator[](int i) { return samples[i]; }

    float average() const { return sum / std::min(numItems, size); }

    void addSample(float s) {
        index = (index + 1) % size;
        numItems++;
        sum += s;

        if (numItems < size) {
            samples[index] = s;
        } else {
            float& old = samples[index];
            sum -= old;
            old = s;
        }
    }
};

typedef std::pair<std::string, Samples> SamplePair;
struct Profiler {
    std::map<std::string, Samples> _acc;

    void addSample(const std::string& name, const std::string& filename,
                   float amt) {
        if (_acc.find(name) == _acc.end()) {
            _acc.insert(std::make_pair(name, Samples()));
            _acc[name].setFilename(filename);
        }
        _acc[name].addSample(amt);
    }
};
static Profiler profiler__DO_NOT_USE;

struct prof {
    std::string filename;
    std::string name;
    std::chrono::high_resolution_clock::time_point start;
#if defined(SUPER_ENGINE_PROFILING_DISABLED)
    prof(const char*, const char*) {}
    ~prof() {}
#else
    prof(const std::string& fileloc, const std::string& n)
        : filename(fileloc), name(n) {
        start = std::chrono::high_resolution_clock::now();
    }
    ~prof() {
        auto end = std::chrono::high_resolution_clock::now();
        long long start_as_ms =
            std::chrono::time_point_cast<std::chrono::nanoseconds>(start)
                .time_since_epoch()
                .count();
        long long end_as_ms =
            std::chrono::time_point_cast<std::chrono::nanoseconds>(end)
                .time_since_epoch()
                .count();
        float duration = (end_as_ms - start_as_ms);
        profiler__DO_NOT_USE.addSample(name, filename, duration);
    }
#endif
};

inline const std::string computeFileLocation(const std::string& file,
                                             int lineNum) {
    return fmt::format("{}:{} ", file, lineNum);
}

// TODO: Ive noticed that every fifth time we start the program
// it gets stuck before rendering anything
// I was able to repro once inside the debugger
// and it seems like its being caused by this function
// when called in app
//
// super.exe`computeMethodName(function="run", prettyFunction="int App::run()",
// extra="") at time.h:117:12
//
// seems like the formatting is the problem
//

// https://stackoverflow.com/a/29856690
inline const std::string computeMethodName(const std::string& function,
                                           const std::string& prettyFunction,
                                           const char* extra) {
    // If the input is a constructor, it gets the beginning of
    // the class name, not of the method. That's why later on we
    // have to search for the first parenthesys
    size_t locFunName = prettyFunction.find(function);
    size_t begin = prettyFunction.rfind(" ", locFunName) + 1;
    // Adding function.length() make
    // this faster and also allows to
    // handle operator parenthesis!
    size_t end = prettyFunction.find("(", locFunName + function.length());
    auto suffix = prettyFunction[end + 1] == ')' ? "()" : "(...)";
    return fmt::format("{}{}{}", prettyFunction.substr(begin, end - begin),
                       suffix, extra);
}

#if defined(SUPER_ENGINE_PROFILING_DISABLED)
#define __PROFILE_FUNC__ "", ""
#define __PROFILE_LOC__(x) "", ""
#else
#define __PROFILE_FUNC__                     \
    computeFileLocation(__FILE__, __LINE__), \
        computeMethodName(__FUNCTION__, __PRETTY_FUNCTION__, "")

#define __PROFILE_LOC__(x)                   \
    computeFileLocation(__FILE__, __LINE__), \
        computeMethodName(__FUNCTION__, __PRETTY_FUNCTION__, x)
#endif
