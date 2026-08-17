// domain/logic/validator/structure/structure_validator.cpp
#include "domain/logic/validator/structure/structure_validator.hpp"

#include <algorithm>
#include <cctype>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_set>

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
constexpr int kSecondsPerMinute = 60;
constexpr int kSecondsPerDay = 24 * 60 * kSecondsPerMinute;
constexpr int kPointBoundaryWrapThresholdSeconds = 4 * 60 * kSecondsPerMinute;

auto IsLeap(int year) -> bool {
  return (year % kYearDivisor4 == 0 &&
          (year % kYearDivisor100 != 0 || year % kYearDivisor400 == 0));
}

[[nodiscard]] auto ParseFlexibleHhmmss(std::string_view time_value)
    -> std::optional<int> {
  std::string digits;
  digits.reserve(6);
  for (const char kValue : time_value) {
    if (std::isdigit(static_cast<unsigned char>(kValue)) != 0) {
      digits.push_back(kValue);
    }
  }
  if (digits.length() == 4) {
    digits.append("00");
  }
  if (digits.length() != 6) {
    return std::nullopt;
  }

  try {
    const int kHour = std::stoi(digits.substr(0, 2));
    const int kMinute = std::stoi(digits.substr(2, 2));
    const int kSecond = std::stoi(digits.substr(4, 2));
    if (kHour < 0 || kHour >= 24 || kMinute < 0 || kMinute >= 60 ||
        kSecond < 0 || kSecond >= 60) {
      return std::nullopt;
    }
    return ((kHour * 60) + kMinute) * kSecondsPerMinute + kSecond;
  } catch (const std::exception&) {
    return std::nullopt;
  }
}

[[nodiscard]] auto IsPlausiblePointBoundaryWrap(int previous_raw_minutes,
                                                int current_raw_minutes)
    -> bool {
  return previous_raw_minutes > current_raw_minutes &&
         (previous_raw_minutes - current_raw_minutes) >=
             kPointBoundaryWrapThresholdSeconds;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
[[nodiscard]] auto ExpandRelativeToBoundary(int raw_minutes,
                                            int boundary_minutes,
                                            bool allow_equal)
    -> std::optional<int> {
  // Expand authored boundary values against the last known boundary so
  // point-style midnight wraps stay monotonic, while short backward moves still
  // fail as overlap. Explicit interval end wrapping is handled separately by
  // ExpandIntervalEndRelativeToStart and does not use this heuristic.
  const int kDayOffset = boundary_minutes / kSecondsPerDay;
  const int kBoundaryRawMinutes = boundary_minutes % kSecondsPerDay;
  int candidate = raw_minutes + (kDayOffset * kSecondsPerDay);

  if (raw_minutes > kBoundaryRawMinutes) {
    return candidate;
  }
  if (raw_minutes == kBoundaryRawMinutes) {
    if (allow_equal) {
      return boundary_minutes;
    }
    return std::nullopt;
  }

  if (!IsPlausiblePointBoundaryWrap(kBoundaryRawMinutes, raw_minutes)) {
    return std::nullopt;
  }
  candidate += kSecondsPerDay;
  if (!allow_equal && candidate <= boundary_minutes) {
    return std::nullopt;
  }
  return candidate;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
[[nodiscard]] auto ExpandIntervalEndRelativeToStart(int end_raw_minutes,
                                                    int start_minutes)
    -> std::optional<int> {
  const int kDayOffset = start_minutes / kSecondsPerDay;
  const int kStartRawMinutes = start_minutes % kSecondsPerDay;
  int candidate = end_raw_minutes + (kDayOffset * kSecondsPerDay);

  if (end_raw_minutes == kStartRawMinutes) {
    return std::nullopt;
  }
  if (end_raw_minutes < kStartRawMinutes) {
    candidate += kSecondsPerDay;
  }
  return candidate;
}

[[nodiscard]] auto ContainsIntervalEvent(const std::vector<DailyLog>& days)
    -> bool {
  return std::ranges::any_of(days, [](const DailyLog& day) {
    return std::ranges::any_of(day.rawEvents, [](const RawEvent& raw_event) {
      return raw_event.kind == RawEventKind::Interval;
    });
  });
}

[[nodiscard]] auto ShouldSeedTimelineFromDayBoundary(const DailyLog& day)
    -> bool {
  if (day.getupTime.empty()) {
    return false;
  }
  return day.rawEvents.empty() ||
         day.rawEvents.front().kind != RawEventKind::Interval;
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

void ValidateActivityDuration(const DailyLog& day,
                              std::vector<Diagnostic>& diagnostics) {
  constexpr int kSecondsPerHour = 60 * 60;
  constexpr int kMaxActivityDurationSeconds = 16 * kSecondsPerHour;
  constexpr std::string_view kAllowLongToken = "@allow-long";

  for (const auto& activity : day.processedActivities) {
    if (!activity.HasValidBoundaryShape()) {
      diagnostics.push_back(
          {.severity = DiagnosticSeverity::kError,
           .code = "activity.record.invalid_boundary_shape",
           .message =
               "In file for date " + day.date +
               ": Activity record boundaries do not match its record kind "
               "(start=" +
               (activity.start_time_str.empty() ? "N/A"
                                                : activity.start_time_str) +
               ", end=" +
               (activity.end_time_str.empty() ? "N/A" : activity.end_time_str) +
               ", project=" + activity.project_path + ").",
           .source_span = activity.source_span});
      continue;
    }

    if (activity.kind == ActivityRecordKind::kEndOnly) {
      // An end-only activity is a valid authored fact whose start boundary is
      // unavailable. It is visible in activity/count queries but has no
      // duration to validate or aggregate.
      continue;
    }

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

void ValidateWakeKeywordPosition(
    const DailyLog& day, const std::unordered_set<std::string>& wake_keywords,
    std::vector<Diagnostic>& diagnostics) {
  if (wake_keywords.empty() || day.rawEvents.empty()) {
    return;
  }

  // Parser only establishes first-event semantics (`getupTime` or
  // `isContinuation`). The formal ingest rule "wake keyword must be the first
  // semantic event of the day" is enforced here during logic validation.
  for (size_t index = 1; index < day.rawEvents.size(); ++index) {
    const auto& raw_event = day.rawEvents[index];
    if (!wake_keywords.contains(raw_event.description)) {
      continue;
    }

    if (raw_event.kind == RawEventKind::Interval) {
      diagnostics.push_back(
          {.severity = DiagnosticSeverity::kError,
           .code = "wake.keyword.interval_not_allowed",
           .message = "In file for date " + day.date +
                      ": Wake keyword activity '" + raw_event.description +
                      "' cannot be authored as an interval event.",
           .source_span = raw_event.source_span});
      continue;
    }

    diagnostics.push_back(
        {.severity = DiagnosticSeverity::kError,
         .code = "wake.keyword.not_first_event",
         .message = "In file for date " + day.date +
                    ": Wake keyword activity '" + raw_event.description +
                    "' must appear only as the first event of the day.",
         .source_span = raw_event.source_span});
  }
}

}  // namespace

auto AnalyzeMixedTimeline(const DailyLog& day,
                          const std::unordered_set<std::string>& wake_keywords)
    -> MixedTimelineAnalysis {
  MixedTimelineAnalysis analysis;
  analysis.event_bounds.resize(day.rawEvents.size());
  std::optional<int> last_known_boundary_minutes;
  if (ShouldSeedTimelineFromDayBoundary(day)) {
    last_known_boundary_minutes = ParseFlexibleHhmmss(day.getupTime);
  }

  for (std::size_t index = 0; index < day.rawEvents.size(); ++index) {
    const auto& raw_event = day.rawEvents[index];
    auto& bounds = analysis.event_bounds[index];
    const bool kIsWake = wake_keywords.contains(raw_event.description) &&
                         raw_event.kind == RawEventKind::Point;
    if (kIsWake) {
      if (!last_known_boundary_minutes.has_value()) {
        last_known_boundary_minutes = ParseFlexibleHhmmss(raw_event.endTimeStr);
      }
      continue;
    }

    const std::optional<int> kEndMinutes =
        ParseFlexibleHhmmss(raw_event.endTimeStr);
    if (!kEndMinutes.has_value()) {
      continue;
    }

    if (raw_event.kind == RawEventKind::Interval) {
      if (!raw_event.startTimeStr.has_value()) {
        analysis.issues.push_back(
            {.event_index = index,
             .code = MixedTimelineIssueCode::kMissingIntervalStart});
        continue;
      }

      if (wake_keywords.contains(raw_event.description)) {
        analysis.issues.push_back(
            {.event_index = index,
             .code = MixedTimelineIssueCode::kWakeIntervalNotAllowed});
      }

      const std::optional<int> kStartMinutes =
          ParseFlexibleHhmmss(*raw_event.startTimeStr);
      if (!kStartMinutes.has_value()) {
        continue;
      }

      if (*kStartMinutes == *kEndMinutes) {
        analysis.issues.push_back(
            {.event_index = index,
             .code = MixedTimelineIssueCode::kInvalidIntervalRange});
        continue;
      }

      int expanded_start_minutes = *kStartMinutes;
      bounds.previous_end_timeline_seconds = last_known_boundary_minutes;
      if (last_known_boundary_minutes.has_value()) {
        const std::optional<int> kExpandedStart = ExpandRelativeToBoundary(
            *kStartMinutes, *last_known_boundary_minutes, true);
        if (!kExpandedStart.has_value()) {
          analysis.issues.push_back(
              {.event_index = index, .code = MixedTimelineIssueCode::kOverlap});
          continue;
        }
        expanded_start_minutes = *kExpandedStart;
      }

      const std::optional<int> kExpandedEnd = ExpandIntervalEndRelativeToStart(
          *kEndMinutes, expanded_start_minutes);
      if (!kExpandedEnd.has_value()) {
        analysis.issues.push_back(
            {.event_index = index,
             .code = MixedTimelineIssueCode::kInvalidIntervalRange});
        continue;
      }
      bounds.participates_in_timeline = true;
      bounds.start_timeline_seconds = expanded_start_minutes;
      bounds.end_timeline_seconds = kExpandedEnd;
      last_known_boundary_minutes = kExpandedEnd;
      continue;
    }

    int expanded_end_minutes = *kEndMinutes;
    bounds.previous_end_timeline_seconds = last_known_boundary_minutes;
    if (last_known_boundary_minutes.has_value()) {
      const std::optional<int> kExpandedEnd = ExpandRelativeToBoundary(
          *kEndMinutes, *last_known_boundary_minutes, false);
      if (!kExpandedEnd.has_value()) {
        analysis.issues.push_back(
            {.event_index = index, .code = MixedTimelineIssueCode::kOverlap});
        continue;
      }
      expanded_end_minutes = *kExpandedEnd;
    }

    bounds.participates_in_timeline = true;
    bounds.start_timeline_seconds = expanded_end_minutes;
    bounds.end_timeline_seconds = expanded_end_minutes;
    last_known_boundary_minutes = expanded_end_minutes;
  }

  std::optional<int> next_start;
  for (std::size_t index = analysis.event_bounds.size(); index-- > 0;) {
    auto& bounds = analysis.event_bounds[index];
    if (!bounds.participates_in_timeline) {
      continue;
    }
    bounds.next_start_timeline_seconds = next_start;
    next_start = bounds.start_timeline_seconds;
  }
  return analysis;
}

namespace {

void ValidateMixedTimeline(const DailyLog& day,
                           const std::unordered_set<std::string>& wake_keywords,
                           std::vector<Diagnostic>& diagnostics) {
  const auto kAnalysis = AnalyzeMixedTimeline(day, wake_keywords);
  for (const auto& issue : kAnalysis.issues) {
    const auto& event = day.rawEvents[issue.event_index];
    std::string code;
    std::string message;
    switch (issue.code) {
      case MixedTimelineIssueCode::kMissingIntervalStart:
        code = "timeline.interval.missing_start";
        message = "Interval event is missing an explicit start time.";
        break;
      case MixedTimelineIssueCode::kInvalidIntervalRange:
        code = "timeline.interval.invalid_range";
        message = "Interval event must not have zero duration.";
        break;
      case MixedTimelineIssueCode::kOverlap:
        code = "timeline.event.overlap";
        message = event.kind == RawEventKind::Interval
                      ? "Interval event overlaps an earlier recorded boundary."
                      : "Point event must end after the last known boundary.";
        break;
      case MixedTimelineIssueCode::kWakeIntervalNotAllowed:
        code = "wake.keyword.interval_not_allowed";
        message = "Wake keyword activity '" + event.description +
                  "' cannot be authored as an interval event.";
        break;
    }
    diagnostics.push_back(
        {.severity = DiagnosticSeverity::kError,
         .code = std::move(code),
         .message = "In file for date " + day.date + ": " + message,
         .source_span = event.source_span});
  }
}

void ValidateDateContinuity(const std::vector<DailyLog>& days,
                            std::vector<Diagnostic>& diagnostics,
                            DateCheckMode mode) {
  if (mode == DateCheckMode::kNone || days.empty()) {
    return;
  }
  if (ContainsIntervalEvent(days)) {
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

StructValidator::StructValidator(DateCheckMode mode,
                                 std::vector<std::string> wake_keywords)
    : date_check_mode_(mode),
      wake_keywords_(wake_keywords.begin(), wake_keywords.end()) {}

auto StructValidator::Validate(const std::string& /*filename*/,
                               const std::vector<DailyLog>& days,
                               std::vector<Diagnostic>& diagnostics) -> bool {
  ValidateDateContinuity(days, diagnostics, date_check_mode_);
  for (const auto& day : days) {
    ValidateActivityDuration(day, diagnostics);
    ValidateWakeKeywordPosition(day, wake_keywords_, diagnostics);
    ValidateMixedTimeline(day, wake_keywords_, diagnostics);
  }
  return !std::ranges::any_of(
      diagnostics, [](const Diagnostic& diagnostic) -> bool {
        return diagnostic.severity == DiagnosticSeverity::kError;
      });
}

}  // namespace validator::structure
