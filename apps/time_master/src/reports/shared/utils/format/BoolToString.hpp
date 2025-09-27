// reports/shared/utils/format/BoolToString.hpp
#ifndef BOOL_TO_STRING_HPP
#define BOOL_TO_STRING_HPP

#include <string>

/**
 * @brief 将表示布尔值的字符串 ("0" 或 "1") 转换为相应的文本形式 ("false" 或 "true")。
 * * @param val 要转换的字符串，期望为 "0" 或 "1"。
 * @return 如果输入为 "1"，则返回 "true"；否则返回 "false"。
 */
std::string bool_to_string(const std::string& val);

#endif // BOOL_TO_STRING_HPP