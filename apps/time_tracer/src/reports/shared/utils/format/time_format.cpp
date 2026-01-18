// reports/shared/utils/format/time_format.cpp
#include "time_format.hpp"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

std::string time_format_duration(long long total_seconds, int avg_days) {
    if (total_seconds == 0) {
        if (avg_days > 1) return "0m (average: 0m/day)";
        return "0m";
    }

    long long seconds_per_day = (avg_days > 1) ? (total_seconds / avg_days) : total_seconds;

    auto format_single_duration = [](long long s) {
        if (s == 0) return std::string("0m");
        long long h = s / 3600;
        long long m = (s % 3600) / 60;
        std::stringstream ss;
        if (h > 0) {
            ss << h << "h";
        }
        if (m > 0 || h == 0) {
            if (h > 0) ss << " "; // Add space between h and m
            ss << m << "m";
        }
        return ss.str();
    };

    std::string main_duration_str = format_single_duration(total_seconds);

    if (avg_days > 1) {
        std::string avg_duration_str = format_single_duration(seconds_per_day);
        main_duration_str += " (average: " + avg_duration_str + "/day)";
    }
    return main_duration_str;
}

// [核心修改] 适配 YYYY-MM-DD 格式
std::string add_days_to_date_str(std::string date_str, int days) {
    // 预期输入: "2025-01-01" (10 chars)
    if (date_str.length() != 10) return date_str;

    // YYYY-MM-DD
    // 0123456789
    int year = std::stoi(date_str.substr(0, 4));
    int month = std::stoi(date_str.substr(5, 2)); // 跳过索引4的 '-'
    int day = std::stoi(date_str.substr(8, 2));   // 跳过索引7的 '-'

    std::tm t{};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day + days;
    // 标准化时间结构，自动处理进位/借位（例如1月32日变成2月1日）
    std::mktime(&t);

    std::stringstream ss;
    // [修改] 输出格式化为标准格式
    ss << std::put_time(&t, "%Y-%m-%d");
    return ss.str();
}

// [核心修改] 适配 YYYY-MM-DD 格式
std::string get_current_date_str() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    // [修改] 输出格式化为标准格式
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
    return ss.str();
}