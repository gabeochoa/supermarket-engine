
#pragma once

#include <codecvt>
#include <locale>
#include <regex>
#include <string>
#include <vector>

using convert_t = std::codecvt_utf8<wchar_t>;
static std::wstring_convert<convert_t, wchar_t> strconverter;

inline std::string to_string(std::wstring wstr) {
    try {
        return strconverter.to_bytes(wstr);
    } catch (...) {
        return std::string();
    }
}

inline std::wstring to_wstring(std::string str) {
    try {
        return strconverter.from_bytes(str);
    } catch (...) {
        return std::wstring();
    }
}

// string split
inline std::vector<std::string> split(const std::string& input,
                                      const std::string& regex) {
    // passing -1 as the submatch index parameter performs splitting
    std::regex re(regex);
    std::sregex_token_iterator first{input.begin(), input.end(), re, -1};
    std::sregex_token_iterator last;
    return std::vector<std::string>{first, last};
}

inline std::string nameFromFilePath(const std::string& filepath) {
    auto lastSlash = filepath.find_last_of("/\\");
    lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
    auto lastDot = filepath.rfind(".");
    auto count = lastDot == std::string::npos ? filepath.size() - lastSlash
                                              : lastDot - lastSlash;
    return filepath.substr(lastSlash, count);
}

