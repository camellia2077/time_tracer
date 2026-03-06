// domain/logic/validator/structure/structure_validator.cpp
#include "domain/logic/validator/structure/structure_validator.hpp"

#include <optional>
#include <set>
#include <string>
#include <string_view>

namespace validator::structure {
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

void ValidateActivityCount(const DailyLog& day,
                           std::vector<Diagnostic>& diagnostics) {
  if (day.processedActivities.size() < 2) {
    diagnostics.push_back(
        {.severity = DiagnosticSeverity::kError,
         .code = "activity.count.too_few",
         .message = "In file for date " + day.date +
                    ": The day has less than 2 activities. This may cause "
                    "issues with 'sleep' activity generation.",
         .source_span = day.source_span});
  }
}

void ValidateActivityDuration(const DailyLog& day,
                              std::vector<Diagnostic>& diagnostics) {
  constexpr int kSecondsPerHour = 60 * 60;
  constexpr int kMaxActivityDurationSeconds = 16 * kSecondsPerHour;
  constexpr std::string_view kAllowLongToken = "@allow-long";

  for (const auto& activity : day.processedActivities) {
    const int kDurationMinutes =
        activity.duration_seconds > 0 ? (activity.duration_seconds / 60) : 0;
    const int kDurationHours = kDurationMinutes / 60;
    const int kDurationRemainingMinutes = kDurationMinutes % 60;
    const std::string kDurationText =
        std::to_string(kDurationHours) + "h " +
        std::to_string(kDurationRemainingMinutes) + "m";

    if (activity.duration_seconds <= 0) {
      const std::string kStartTime =
          activity.start_time_str.empty() ? "N/A" : activity.start_time_str;
      const std::string kEndTime =
          activity.end_time_str.empty() ? "N/A" : activity.end_time_str;
      const bool kSameTime = !activity.start_time_str.empty() &&
                             (activity.start_time_str == activity.end_time_str);
      const std::string kExtraHint =
          kSameTime ? " (start_time equals end_time)" : "";
      diagnostics.push_back(
          {.severity = DiagnosticSeverity::kError,
           .code = "activity.duration.zero",
           .message =
               "In file for date " + day.date +
               ": Activity duration must be positive (start=" + kStartTime +
               ", end=" + kEndTime + ", duration=" + kDurationText +
               ", project=" + activity.project_path + ")" + kExtraHint + ".",
           .source_span = activity.source_span});
      continue;
    }

    const bool kAllowLong =
        activity.remark.has_value() &&
        (activity.remark.value().find(kAllowLongToken) != std::string::npos);
    if (!kAllowLong &&
        activity.duration_seconds > kMaxActivityDurationSeconds) {
      const std::string kStartTime =
          activity.start_time_str.empty() ? "N/A" : activity.start_time_str;
      const std::string kEndTime =
          activity.end_time_str.empty() ? "N/A" : activity.end_time_str;
      diagnostics.push_back(
          {.severity = DiagnosticSeverity::kError,
           .code = "activity.duration.too_long",
           .message = "In file for date " + day.date +
                      ": Activity duration exceeds 16 hours (start=" +
                      kStartTime + ", end=" + kEndTime + ", duration=" +
                      kDurationText + ", project=" + activity.project_path +
                      "). Use @allow-long in remark to bypass.",
           .source_span = activity.source_span});
    }
  }
}

void ValidateDateContinuity(const std::vector<DailyLog>& days,
                            std::vector<Diagnostic>& diagnostics,
                            DateCheckMode mode) {
  if (mode == DateCheckMode::kNone || days.empty()) {
    return;
  }

  const auto& first_day = days[0];
  if (first_day.date.length() != kDateStringLength) {
    return;
  }

  std::string yyyy_mm = first_day.date.substr(0, kIsoYearMonthLength);
  int year = std::stoi(first_day.date.substr(0, kIsoYearLength));
  int month =
      std::stoi(first_day.date.substr(kIsoMonthOffset, kIsoMonthLength));

  std::set<int> days_found;
  for (const auto& day : days) {
    if (day.date.starts_with(yyyy_mm) &&
        day.date.length() == kDateStringLength) {
      days_found.insert(
          std::stoi(day.date.substr(kIsoDayOffset, kIsoDayLength)));
    }
  }

  int check_until = DaysInMonth(year, month);
  if (mode == DateCheckMode::kContinuity) {
    if (days_found.empty()) {
      return;
    }
    check_until = *days_found.rbegin();
  }

  for (int day_val = 1; day_val <= check_until; ++day_val) {
    if (!days_found.contains(day_val)) {
      std::string missing_date = yyyy_mm + "-" +
                                 (day_val < kSingleDigitThreshold ? "0" : "") +
                                 std::to_string(day_val);
      std::string error_msg =
          "Missing date detected in month " + yyyy_mm + ": " + missing_date;
      if (mode == DateCheckMode::kContinuity) {
        error_msg += " (Continuity Check)";
      } else {
        error_msg += " (Completeness Check)";
      }
      diagnostics.push_back({.severity = DiagnosticSeverity::kError,
                             .code = "date.continuity.missing",
                             .message = std::move(error_msg),
                             .source_span = std::nullopt});
    }
  }
}
}  // namespace

StructValidator::StructValidator(DateCheckMode mode) : date_check_mode_(mode) {}

auto StructValidator::Validate(const std::string& /*filename*/,
                               const std::vector<DailyLog>& days,
                               std::vector<Diagnostic>& diagnostics) -> bool {
  ValidateDateContinuity(days, diagnostics, date_check_mode_);
  for (const auto& day : days) {
    ValidateActivityCount(day, diagnostics);
    ValidateActivityDuration(day, diagnostics);
  }
  return diagnostics.empty();
}

}  // namespace validator::structure
