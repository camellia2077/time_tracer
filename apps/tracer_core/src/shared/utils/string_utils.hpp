// shared/utils/string_utils.hpp
#ifndef SHARED_UTILS_STRING_UTILS_H_
#define SHARED_UTILS_STRING_UTILS_H_

#include <sstream>
#include <string>
#include <vector>

/**
 * @brief 去除字符串两端的空白字符。
 * @param str 输入的字符串。
 * @return 返回去除两端空白字符后的新字符串。
 */
inline auto Trim(const std::string& str) -> std::string {
  const std::string kWhitespace = " \n\r\t\f\v";
  size_t first = str.find_first_not_of(kWhitespace);
  if (std::string::npos == first) {
    return "";
  }
  size_t last = str.find_last_not_of(kWhitespace);
  return str.substr(first, (last - first + 1));
}

/**
 * @brief 根据指定的分隔符拆分字符串。
 * @param s 需要拆分的字符串。
 * @param delimiter 分隔符。
 * @return 包含所有子字符串的向量。
 */
inline auto SplitString(const std::string& str, char delimiter)
    -> std::vector<std::string> {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream token_stream(str);
  while (std::getline(token_stream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

#endif  // SHARED_UTILS_STRING_UTILS_H_
