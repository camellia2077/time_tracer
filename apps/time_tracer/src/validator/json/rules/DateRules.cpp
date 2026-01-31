// validator/json/rules/DateRules.cpp
#include "DateRules.hpp"

#include <iostream>
#include <map>
#include <string>

namespace validator::json {

namespace {
auto IsLeap(int year) -> bool {
  return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}
auto DaysInMonth(int year, int month) -> int {
  if (month < 1 || month > 12) {
    return 0;
  }
  if (month == 2) {
    return IsLeap(year) ? 29 : 28;
  }
  if (month == 4 || month == 6 || month == 9 || month == 11) {
    return 30;
  }
  return 31;
}
}  // namespace

// [修改] 参数类型 json -> nlohmann::json
void validateDateContinuity(const nlohmann::json& days_array,
                            std::set<Error>& errors, DateCheckMode mode) {
  if (mode == DateCheckMode::None) {
    return;
  }
  if (days_array.empty()) {
    return;
  }

  // [修改] json::object() -> nlohmann::json::object()
  const auto& first_day_headers =
      days_array[0].value("headers", nlohmann::json::object());
  std::string first_date = first_day_headers.value("date", "");

  if (first_date.length() != 10) {
    return;
  }

  std::string yyyy_mm = first_date.substr(0, 7);
  int year = std::stoi(first_date.substr(0, 4));
  int month = std::stoi(first_date.substr(5, 2));

  std::map<std::string, std::set<int>> month_day_map;
  for (const auto& day : days_array) {
    // [修改] json::object() -> nlohmann::json::object()
    const auto& headers = day.value("headers", nlohmann::json::object());
    std::string date_str = headers.value("date", "");

    if (date_str.starts_with(yyyy_mm) && date_str.length() == 10) {
      month_day_map[yyyy_mm].insert(std::stoi(date_str.substr(8, 2)));
    }
  }

  const auto& days_found = month_day_map[yyyy_mm];
  int num_days_in_this_month = DaysInMonth(year, month);

  int check_until = num_days_in_this_month;

  if (mode == DateCheckMode::Continuity) {
    if (days_found.empty()) {
      return;
    }
    check_until = *days_found.rbegin();
  }

  for (int d = 1; d <= check_until; ++d) {
    if (!days_found.contains(d)) {
      std::string missing_date_str =
          yyyy_mm + "-" + (d < 10 ? "0" : "") + std::to_string(d);
      std::string error_msg =
          "Missing date detected in month " + yyyy_mm + ": " + missing_date_str;

      if (mode == DateCheckMode::Continuity) {
        error_msg += " (Continuity Check)";
      } else {
        error_msg += " (Completeness Check)";
      }

      errors.insert({0, error_msg, ErrorType::DateContinuity});
    }
  }
}

}  // namespace validator::json
