// common/utils/string_utils.cpp
#include "string_utils.hpp"

#include <sstream>

auto trim(const std::string& str) -> std::string {
  const std::string kWhitespace = " \n\r\t\f\v";
  size_t first = str.find_first_not_of(kWhitespace);
  if (std::string::npos == first) {
    return "";
  }
  size_t last = str.find_last_not_of(kWhitespace);
  return str.substr(first, (last - first + 1));
}

auto split_string(const std::string& s, char delimiter)
    -> std::vector<std::string> {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream token_stream(s);
  while (std::getline(token_stream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}