// domain/utils/time_utils.hpp
#ifndef DOMAIN_UTILS_TIME_UTILS_H_
#define DOMAIN_UTILS_TIME_UTILS_H_

#include <string>

/**
 * @brief 将 HH:MM 或 HHMM 格式的时间字符串转换为自午夜以来的秒数。
 */
auto TimeStrToSeconds(const std::string& time_str_in) -> int;

// --- [新增] ---
/**
 * @brief 将日期字符串标准化为 YYYY-MM-DD。
 * 支持输入: "20250101" 或 "2025-01-01"
 */
auto NormalizeToDateFormat(const std::string& input) -> std::string;

/**
 * @brief 将月份字符串标准化为 YYYY-MM。
 * 支持输入: "202501" 或 "2025-01"
 */
auto NormalizeToMonthFormat(const std::string& input) -> std::string;

#endif  // DOMAIN_UTILS_TIME_UTILS_H_