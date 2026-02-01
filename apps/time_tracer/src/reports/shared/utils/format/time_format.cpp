// reports/shared/utils/format/time_format.cpp
#include "time_format.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace {
constexpr long long kSecondsInHour = 3600;
constexpr long long kSecondsInMinute = 60;
constexpr int kDateStringLength = 10;
constexpr int kMonthOffset = 5;
constexpr int kDayOffset = 8;
constexpr int kTmYearBase = 1900;
}  // namespace

auto time_format_duration(long long total_seconds, int avg_days)
    -> std::string {
  if (total_seconds == 0) {
    if (avg_days > 1) {
      return "0m (average: 0m/day)";
    }
    return "0m";
  }

  long long seconds_per_day =
      (avg_days > 1) ? (total_seconds / avg_days) : total_seconds;

  auto format_single_duration =
      [](long long total_seconds) -> std::basic_string<char> {
    if (total_seconds == 0) {
      return {"0m"};
    }
    long long hours = total_seconds / kSecondsInHour;
    long long minutes = (total_seconds % kSecondsInHour) / kSecondsInMinute;
    std::stringstream formatted_ss;
    if (hours > 0) {
      formatted_ss << hours << "h";
    }
    if (minutes > 0 || hours == 0) {
      if (hours > 0) {
        formatted_ss << " ";  // Add space between h and m
      }
      formatted_ss << minutes << "m";
    }
    return formatted_ss.str();
  };

  std::string main_duration_str = format_single_duration(total_seconds);

  if (avg_days > 1) {
    std::string avg_duration_str = format_single_duration(seconds_per_day);
    main_duration_str += " (average: " + avg_duration_str + "/day)";
  }
  return main_duration_str;
}

// [核心修改] 适配 YYYY-MM-DD 格式
auto add_days_to_date_str(std::string date_str, int days) -> std::string {
  // 预期输入: "2025-01-01" (10 chars)
  if (date_str.length() != static_cast<size_t>(kDateStringLength)) {
    return date_str;
  }

  // YYYY-MM-DD
  // 0123456789
  int year = std::stoi(date_str.substr(0, 4));
  int month = std::stoi(date_str.substr(kMonthOffset, 2));  // 跳过索引4的 '-'
  int day = std::stoi(date_str.substr(kDayOffset, 2));      // 跳过索引7的 '-'

  std::tm time_info{};
  time_info.tm_year = year - kTmYearBase;
  time_info.tm_mon = month - 1;
  time_info.tm_mday = day + days;
  // 标准化时间结构，自动处理进位/借位（例如1月32日变成2月1日）
  std::mktime(&time_info);

  std::stringstream formatted_ss;
  // [修改] 输出格式化为标准格式
  formatted_ss << std::put_time(&time_info, "%Y-%m-%d");
  return formatted_ss.str();
}

// [核心修改] 适配 YYYY-MM-DD 格式
auto get_current_date_str() -> std::string {
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  std::stringstream formatted_ss;
  // [修改] 输出格式化为标准格式
  formatted_ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
  return formatted_ss.str();
}