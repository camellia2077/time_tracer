// infrastructure/reports/shared/utils/format/time_format.hpp
#ifndef REPORTS_SHARED_UTILS_FORMAT_TIME_FORMAT_H_
#define REPORTS_SHARED_UTILS_FORMAT_TIME_FORMAT_H_

#include <string>

#include "infrastructure/reports/shared/api/shared_api.hpp"

/**
 * @brief 将总秒数格式化为 "XhYYm" 或 "YYm" 的字符串，并可选择计算平均值。
 * @param total_seconds 总秒数。
 * @param avg_days 用于计算平均值的天数，默认为1。
 * @return 格式化后的时长字符串。
 */
REPORTS_SHARED_API auto TimeFormatDuration(long long total_seconds,
                                           int avg_days = 1) -> std::string;

/**
 * @brief 为指定的日期字符串增加或减少天数。
 * @param date_str 格式为 "YYYYMMDD" 的日期字符串。
 * @param days 要增加（正数）或减少（负数）的天数。
 * @return 计算后的新日期字符串。
 */
REPORTS_SHARED_API auto AddDaysToDateStr(std::string date_str, int days)
    -> std::string;

/**
 * @brief 获取当前系统的日期。
 * @return 格式为 "YYYYMMDD" 的当前日期字符串。
 */
REPORTS_SHARED_API auto GetCurrentDateStr() -> std::string;

#endif  // REPORTS_SHARED_UTILS_FORMAT_TIME_FORMAT_H_
