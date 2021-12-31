
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

    float& operator[](int index) { return samples[index]; }

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

static std::map<std::string, Samples> _acc;
typedef std::pair<std::string, Samples> SamplePair;
struct prof {
    const char* filename;
    const char* name;
    std::chrono::high_resolution_clock::time_point start;

    prof(const char* fileloc, const char* n) : filename(fileloc), name(n) {
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

        std::map<std::string, Samples>::iterator p = _acc.find(name);
        if (p == _acc.end()) {
            _acc.insert(std::make_pair(name, Samples()));
            _acc[name].setFilename(filename);
        }
        _acc[name].addSample(duration);
    }
};

inline const std::string computeFileLocation(const std::string& file,
                                             int lineNum) {
    return fmt::format("{}:{} ", file, lineNum);
}

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

#define __PROFILE_FUNC__                             \
    computeFileLocation(__FILE__, __LINE__).c_str(), \
        computeMethodName(__FUNCTION__, __PRETTY_FUNCTION__, "").c_str()

#define __PROFILE_LOC__(x)                           \
    computeFileLocation(__FILE__, __LINE__).c_str(), \
        computeMethodName(__FUNCTION__, __PRETTY_FUNCTION__, x).c_str()
