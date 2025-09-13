// common/utils/TimeUtils.hpp
#ifndef TIME_UTILS_HPP
#define TIME_UTILS_HPP

#include <string>

// --- [核心修改] ProjectNode 和 ProjectTree 的定义已从此文件移除 ---

// --- 函数声明 ---

/**
 * @brief 将 HH:MM 或 HHMM 格式的时间字符串转换为自午夜以来的秒数。
 * @param time_str_in 输入的时间字符串。
 * @return 对应的秒数。
 */
int time_str_to_seconds(const std::string& time_str_in);

/**
 * @brief 将总秒数格式化为 "XhYYm" 或 "YYm" 的字符串，并可选择计算平均值。
 * @param total_seconds 总秒数。
 * @param avg_days 用于计算平均值的天数，默认为1。
 * @return 格式化后的时长字符串。
 */
std::string time_format_duration(long long total_seconds, int avg_days = 1);

#endif // TIME_UTILS_HPP