// infrastructure/reports/shared/utils/format/time_format.cpp
#include "infrastructure/reports/shared/utils/format/time_format.hpp"

#include <chrono>
#include <ctime>
#include <string>

namespace {
constexpr long long kSecondsInHour = 3600;
constexpr long long kSecondsInMinute = 60;
constexpr int kDateStringLength = 10;
constexpr std::size_t kDurationBufferSize = 32U;
constexpr int kMonthOffset = 5;
constexpr int kDayOffset = 8;
constexpr int kTmYearBase = 1900;
constexpr int kYearWidth = 4;
constexpr int kMonthDayWidth = 2;
}  // namespace

namespace {

auto BuildDurationText(long long total_seconds) -> std::string {
  long long hours = total_seconds / kSecondsInHour;
  long long minutes = (total_seconds % kSecondsInHour) / kSecondsInMinute;

  std::string output;
  output.reserve(kDurationBufferSize);
  output += std::to_string(hours);
  output += "h ";
  output += std::to_string(minutes);
  output += "m";
  return output;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void AppendPaddedNumber(std::string& output, int value, int width) {
  std::string digits = std::to_string(value);
  if (digits.size() < static_cast<size_t>(width)) {
    output.append(static_cast<size_t>(width) - digits.size(), '0');
  }
  output += digits;
}

auto FormatDateTm(const std::tm& time_info) -> std::string {
  std::string output;
  output.reserve(kDateStringLength);
  AppendPaddedNumber(output, time_info.tm_year + kTmYearBase, kYearWidth);
  output += "-";
  AppendPaddedNumber(output, time_info.tm_mon + 1, kMonthDayWidth);
  output += "-";
  AppendPaddedNumber(output, time_info.tm_mday, kMonthDayWidth);
  return output;
}

}  // namespace

auto TimeFormatDuration(long long total_seconds, int avg_days) -> std::string {
  if (total_seconds == 0) {
    if (avg_days > 1) {
      return "0h 0m (average: 0h 0m/day)";
    }
    return "0h 0m";
  }

  long long seconds_per_day =
      (avg_days > 1) ? (total_seconds / avg_days) : total_seconds;

  std::string main_duration_str = BuildDurationText(total_seconds);

  if (avg_days > 1) {
    std::string avg_duration_str = BuildDurationText(seconds_per_day);
    main_duration_str += " (average: " + avg_duration_str + "/day)";
  }
  return main_duration_str;
}

auto AddDaysToDateStr(std::string date_str, int days) -> std::string {
  if (date_str.length() != static_cast<size_t>(kDateStringLength)) {
    return date_str;
  }

  int year = std::stoi(date_str.substr(0, 4));
  int month = std::stoi(date_str.substr(kMonthOffset, 2));
  int day = std::stoi(date_str.substr(kDayOffset, 2));

  std::tm time_info{};
  time_info.tm_year = year - kTmYearBase;
  time_info.tm_mon = month - 1;
  time_info.tm_mday = day + days;
  std::mktime(&time_info);

  return FormatDateTm(time_info);
}

auto GetCurrentDateStr() -> std::string {
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);

  const std::tm* time_info = std::localtime(&in_time_t);
  if (time_info == nullptr) {
    return "";
  }

  return FormatDateTm(*time_info);
}
