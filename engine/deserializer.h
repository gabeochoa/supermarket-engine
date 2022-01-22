
#pragma once

#include <sstream>

#include "external_include.h"

template <typename T>
struct Deserializer {
    std::string input;
    Deserializer(std::string i) : input(i) {}
    virtual operator T() = 0;
};

template <>
struct Deserializer<bool> {
    std::string input;
    Deserializer(std::string i) : input(i) {}
    operator bool() {
        bool b;
        std::istringstream(this->input.c_str()) >> std::boolalpha >> b;
        return b;
    }
};

template <>
struct Deserializer<float> {
    std::string input;
    Deserializer(std::string i) : input(i) {}
    operator float() { return ::atof(this->input.c_str()); }
};

template <>
struct Deserializer<int> {
    std::string input;
    Deserializer(std::string i) : input(i) {}
    operator int() { return ::atoi(this->input.c_str()); }
};

