// queries/shared/utils/format/TimeFormat.hpp
#ifndef TIME_FORMAT_HPP
#define TIME_FORMAT_HPP

#include <string>

/**
 * @brief 将秒数转换为 "h m" 格式的字符串。
 * @param duration_seconds 持续时间，以秒为单位。
 * @return 格式化后的字符串。
 */
std::string time_format_duration_hm(long long duration_seconds);

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