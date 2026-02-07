// domain/logic/validator/json/rules/date_rules.cpp
#include "domain/logic/validator/json/rules/date_rules.hpp"

#include <iostream>
#include <map>
#include <string>

namespace validator::json {

namespace {
constexpr int kDaysInNormalFebruary = 28;
constexpr int kDaysInLeapFebruary = 29;
constexpr int kDaysInSmallMonth = 30;
constexpr int kDaysInBigMonth = 31;
constexpr int kMonthsInYear = 12;

constexpr int kYearDivisor4 = 4;
constexpr int kYearDivisor100 = 100;
constexpr int kYearDivisor400 = 400;

constexpr int kApril = 4;
constexpr int kJune = 6;
constexpr int kSeptember = 9;
constexpr int kNovember = 11;

constexpr int kDateStringLength = 10;
constexpr int kIsoYearLength = 4;
constexpr int kIsoMonthOffset = 5;
constexpr int kIsoMonthLength = 2;
constexpr int kIsoYearMonthLength = 7;
constexpr int kIsoDayOffset = 8;
constexpr int kIsoDayLength = 2;
constexpr int kSingleDigitThreshold = 10;

auto IsLeap(int year) -> bool {
  return (year % kYearDivisor4 == 0 &&
          (year % kYearDivisor100 != 0 || year % kYearDivisor400 == 0));
}
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
auto DaysInMonth(int year, int month) -> int {
  if (month < 1 || month > kMonthsInYear) {
    return 0;
  }
  if (month == 2) {
    return IsLeap(year) ? kDaysInLeapFebruary : kDaysInNormalFebruary;
  }
  if (month == kApril || month == kJune || month == kSeptember ||
      month == kNovember) {
    return kDaysInSmallMonth;
  }
  return kDaysInBigMonth;
}
// NOLINTEND(bugprone-easily-swappable-parameters)
}  // namespace

// [修改] 参数类型 json -> nlohmann::json
void validateDateContinuity(const nlohmann::json& days_array,
                            std::set<Error>& errors, DateCheckMode mode) {
  if (mode == DateCheckMode::kNone) {
    return;
  }
  if (days_array.empty()) {
    return;
  }

  // [修改] json::object() -> nlohmann::json::object()
  const auto& first_day_headers =
      days_array[0].value("headers", nlohmann::json::object());
  std::string first_date = first_day_headers.value("date", "");

  if (first_date.length() != kDateStringLength) {
    return;
  }

  std::string yyyy_mm = first_date.substr(0, kIsoYearMonthLength);
  int year = std::stoi(first_date.substr(0, kIsoYearLength));
  int month = std::stoi(first_date.substr(kIsoMonthOffset, kIsoMonthLength));

  std::map<std::string, std::set<int>> month_day_map;
  for (const auto& day : days_array) {
    // [修改] json::object() -> nlohmann::json::object()
    const auto& headers = day.value("headers", nlohmann::json::object());
    std::string date_str = headers.value("date", "");

    if (date_str.starts_with(yyyy_mm) &&
        date_str.length() == kDateStringLength) {
      month_day_map[yyyy_mm].insert(
          std::stoi(date_str.substr(kIsoDayOffset, kIsoDayLength)));
    }
  }

  const auto& days_found = month_day_map[yyyy_mm];
  int num_days_in_this_month = DaysInMonth(year, month);

  int check_until = num_days_in_this_month;

  if (mode == DateCheckMode::kContinuity) {
    if (days_found.empty()) {
      return;
    }
    check_until = *days_found.rbegin();
  }

  for (int day_val = 1; day_val <= check_until; ++day_val) {
    if (!days_found.contains(day_val)) {
      std::string missing_date_str =
          yyyy_mm + "-" + (day_val < kSingleDigitThreshold ? "0" : "") +
          std::to_string(day_val);
      std::string error_msg =
          "Missing date detected in month " + yyyy_mm + ": " + missing_date_str;

      if (mode == DateCheckMode::kContinuity) {
        error_msg += " (Continuity Check)";
      } else {
        error_msg += " (Completeness Check)";
      }

      errors.insert({0, error_msg, ErrorType::kDateContinuity});
    }
  }
}

}  // namespace validator::json
