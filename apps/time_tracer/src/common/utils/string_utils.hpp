// common/utils/string_utils.hpp
#ifndef COMMON_UTILS_STRING_UTILS_H_
#define COMMON_UTILS_STRING_UTILS_H_

#include <string>
#include <vector>

#include "reports/shared/api/shared_api.hpp"  // <--- [新增]

/**
 * @brief 去除字符串两端的空白字符。
 * @param str 输入的字符串。
 * @return 返回去除两端空白字符后的新字符串。
 */
REPORTS_SHARED_API auto Trim(const std::string& str) -> std::string;

/**
 * @brief 根据指定的分隔符拆分字符串。
 * @param s 需要拆分的字符串。
 * @param delimiter 分隔符。
 * @return 包含所有子字符串的向量。
 */
REPORTS_SHARED_API auto SplitString(const std::string& str, char delimiter)
    -> std::vector<std::string>;

#endif  // COMMON_UTILS_STRING_UTILS_H_