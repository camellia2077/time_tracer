// domain/utils/time_utils.cpp
#include "domain/utils/time_utils.hpp"

#include <algorithm>  // for std::all_of
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace {
constexpr int kTimeFormatLength = 5;
constexpr int kHoursInDay = 24;
constexpr int kMinutesInHour = 60;
constexpr int kSecondsInHour = 3600;
constexpr int kSecondsInMinute = 60;
constexpr int kDateStringLength = 8;
constexpr int kMonthOffset = 4;
constexpr int kDayOffset = 6;
constexpr int kMonthFormatLength = 6;
}  // namespace

auto time_str_to_seconds(const std::string& time_str_in) -> int {
  std::string time_str = time_str_in;
  if (time_str.length() == static_cast<size_t>(kTimeFormatLength - 1) &&
      time_str.find(':') == std::string::npos) {
    bool all_digits = true;
    for (char digit_char : time_str) {
      if (std::isdigit(static_cast<unsigned char>(digit_char)) == 0) {
        all_digits = false;
        break;
      }
    }
    if (all_digits) {
      time_str = time_str.substr(0, 2) + ":" + time_str.substr(2, 2);
    }
  }

  if (time_str.length() != static_cast<size_t>(kTimeFormatLength) ||
      time_str[2] != ':') {
    return 0;
  }
  try {
    int hours = std::stoi(time_str.substr(0, 2));
    int minutes = std::stoi(time_str.substr(3, 2));
    if (hours < 0 || hours >= kHoursInDay || minutes < 0 ||
        minutes >= kMinutesInHour) {
      return 0;
    }
    return (hours * kSecondsInHour) + (minutes * kSecondsInMinute);
  } catch (const std::exception&) {
    return 0;
  }
}

// --- [新增实现] ---

auto normalize_to_date_format(const std::string& input) -> std::string {
  // 如果是 8 位纯数字 (20250101)，转换为 2025-01-01
  if (input.length() == static_cast<size_t>(kDateStringLength) &&
      std::ranges::all_of(input, ::isdigit)) {
    return input.substr(0, 4) + "-" + input.substr(kMonthOffset, 2) + "-" +
           input.substr(kDayOffset, 2);
  }
  // 否则原样返回 (假设已经是 2025-01-01，或者让后续逻辑处理错误)
  return input;
}

auto normalize_to_month_format(const std::string& input) -> std::string {
  // 如果是 6 位纯数字 (202501)，转换为 2025-01
  if (input.length() == static_cast<size_t>(kMonthFormatLength) &&
      std::ranges::all_of(input, ::isdigit)) {
    return input.substr(0, 4) + "-" + input.substr(kMonthOffset, 2);
  }
  return input;
}