// reports/shared/utils/format/bool_to_string.hpp
#ifndef REPORTS_SHARED_UTILS_FORMAT_BOOL_TO_STRING_H_
#define REPORTS_SHARED_UTILS_FORMAT_BOOL_TO_STRING_H_

#include <string>

#include "reports/shared/api/shared_api.hpp"  // <--- [新增]

/**
 * @brief 将表示布尔值的字符串 ("0" 或 "1") 转换为相应的文本形式 ("false" 或
 * "true")。
 * * @param val 要转换的字符串，期望为 "0" 或 "1"。
 * @return 如果输入为 "1"，则返回 "true"；否则返回 "false"。
 */
REPORTS_SHARED_API std::string bool_to_string(const std::string& val);

#endif  // REPORTS_SHARED_UTILS_FORMAT_BOOL_TO_STRING_H_