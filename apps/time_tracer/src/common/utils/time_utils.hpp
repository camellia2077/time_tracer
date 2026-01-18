// common/utils/time_utils.hpp
#ifndef COMMON_UTILS_TIME_UTILS_HPP_
#define COMMON_UTILS_TIME_UTILS_HPP_

#include <string>

/**
 * @brief 将 HH:MM 或 HHMM 格式的时间字符串转换为自午夜以来的秒数。
 */
int time_str_to_seconds(const std::string& time_str_in);

// --- [新增] ---
/**
 * @brief 将日期字符串标准化为 YYYY-MM-DD。
 * 支持输入: "20250101" 或 "2025-01-01"
 */
std::string normalize_to_date_format(const std::string& input);

/**
 * @brief 将月份字符串标准化为 YYYY-MM。
 * 支持输入: "202501" 或 "2025-01"
 */
std::string normalize_to_month_format(const std::string& input);

#endif // COMMON_UTILS_TIME_UTILS_HPP_