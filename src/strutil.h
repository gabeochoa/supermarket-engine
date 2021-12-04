
#pragma once 
#include <string>
#include <vector>
#include <regex>

#include "pch.hpp"

// string split
inline std::vector<std::string> split(const std::string& input,
                               const std::string& regex) {
    // passing -1 as the submatch index parameter performs splitting
    std::regex re(regex);
    std::sregex_token_iterator first{input.begin(), input.end(), re, -1};
    std::sregex_token_iterator last;
    return std::vector<std::string>{first, last};
}

