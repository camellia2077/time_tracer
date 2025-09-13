// common/utils/StringUtils.hpp
#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <string>
#include <vector>

/**
 * @brief 去除字符串两端的空白字符。
 * @param str 输入的字符串。
 * @return 返回去除两端空白字符后的新字符串。
 */
std::string trim(const std::string& str);

/**
 * @brief 根据指定的分隔符拆分字符串。
 * @param s 需要拆分的字符串。
 * @param delimiter 分隔符。
 * @return 包含所有子字符串的向量。
 */
std::vector<std::string> split_string(const std::string& s, char delimiter);

#endif // STRING_UTILS_HPP