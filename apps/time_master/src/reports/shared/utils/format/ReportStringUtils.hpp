// reports/shared/utils/format/ReportStringUtils.hpp
#ifndef REPORT_STRING_UTILS_HPP
#define REPORT_STRING_UTILS_HPP

#include <string>

/**
 * @brief 替换字符串中所有匹配的子串。
 * @param str 原始字符串。
 * @param from 要被替换的子串。
 * @param to 用于替换的子串。
 * @return 返回替换后的新字符串。
 */
std::string replace_all(std::string str, const std::string& from, const std::string& to);

#endif // REPORT_STRING_UTILS_HPP