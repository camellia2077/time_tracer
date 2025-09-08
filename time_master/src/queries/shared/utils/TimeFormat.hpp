// queries/shared/utils/TimeFormat.hpp
#ifndef TIME_FORMAT_HPP
#define TIME_FORMAT_HPP

#include <string>

/**
 * @brief 将秒数转换为 "h m" 格式的字符串。
 * @param duration_seconds 持续时间，以秒为单位。
 * @return 格式化后的字符串。
 */
std::string time_format_duration_hm(long long duration_seconds);

#endif // TIME_FORMAT_HPP