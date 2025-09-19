// common/utils/TimeUtils.hpp
#ifndef TIME_UTILS_HPP
#define TIME_UTILS_HPP

#include <string>

/**
 * @brief 将 HH:MM 或 HHMM 格式的时间字符串转换为自午夜以来的秒数。
 * @param time_str_in 输入的时间字符串。
 * @return 对应的秒数。
 */
int time_str_to_seconds(const std::string& time_str_in);

#endif // TIME_UTILS_HPP