// common/utils/StringUtils.cpp
#include "StringUtils.hpp"
#include <sstream>

std::string trim(const std::string& str) {
    const std::string WHITESPACE = " \n\r\t\f\v";
    size_t first = str.find_first_not_of(WHITESPACE);
    if (std::string::npos == first) {
        return "";
    }
    size_t last = str.find_last_not_of(WHITESPACE);
    return str.substr(first, (last - first + 1));
}

std::vector<std::string> split_string(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}