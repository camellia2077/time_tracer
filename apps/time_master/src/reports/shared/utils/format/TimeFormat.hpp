// reports/shared/utils/format/TimeFormat.hpp
#ifndef TIME_FORMAT_HPP
#define TIME_FORMAT_HPP

#include <string>

/**
 * @brief 将总秒数格式化为 "XhYYm" 或 "YYm" 的字符串，并可选择计算平均值。
 * @param total_seconds 总秒数。
 * @param avg_days 用于计算平均值的天数，默认为1。
 * @return 格式化后的时长字符串。
 */
std::string time_format_duration(long long total_seconds, int avg_days = 1);

/**
 * @brief 为指定的日期字符串增加或减少天数。
 * @param date_str 格式为 "YYYYMMDD" 的日期字符串。
 * @param days 要增加（正数）或减少（负数）的天数。
 * @return 计算后的新日期字符串。
 */
std::string add_days_to_date_str(std::string date_str, int days);

/**
 * @brief 获取当前系统的日期。
 * @return 格式为 "YYYYMMDD" 的当前日期字符串。
 */
std::string get_current_date_str();

#endif // TIME_FORMAT_HPP