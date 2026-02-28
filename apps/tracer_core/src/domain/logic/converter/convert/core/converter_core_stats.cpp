// domain/logic/converter/convert/core/converter_core_stats.cpp
#include <algorithm>
#include <array>
#include <ctime>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string_view>

#include "domain/logic/converter/convert/core/converter_core_internal.hpp"

namespace converter_core_internal {
namespace {

constexpr long long kDailySequenceBase = 1000000;
constexpr int kHoursPerDay = 24;
constexpr int kMinutesPerHour = 60;
constexpr int kSecondsPerMinute = 60;
constexpr int kSecondsPerDay =
    kHoursPerDay * kMinutesPerHour * kSecondsPerMinute;

constexpr size_t kDateStringLength = 10;
constexpr size_t kTimeStringLength = 5;
constexpr size_t kTimeDigitsLength = 4;

constexpr size_t kTimeHourOffset = 0;
constexpr size_t kTimeHourLength = 2;
constexpr size_t kTimeMinuteOffset = 3;
constexpr size_t kTimeMinuteLength = 2;

constexpr size_t kTimeDigitsMinuteOffset = 2;
constexpr size_t kDateTimeMinLength = 16;

auto StringToTimeT(const std::string& datetime_str) -> long long {
  if (datetime_str.length() < kDateTimeMinLength) {
    return 0;
  }
  std::tm time_info = {};
  std::stringstream time_stream(datetime_str);
  time_stream >> std::get_time(&time_info, "%Y-%m-%d %H:%M");
  if (time_stream.fail()) {
    return 0;
  }
  return static_cast<long long>(std::mktime(&time_info));
}

[[nodiscard]] auto ParseHhmmToSeconds(std::string_view time_value)
    -> std::optional<int> {
  if (time_value.length() != kTimeStringLength) {
    return std::nullopt;
  }
  if (time_value[2] != ':') {
    return std::nullopt;
  }

  try {
    const int kHour =
        std::stoi(std::string(time_value.substr(kTimeHourOffset, 2)));
    const int kMinute =
        std::stoi(std::string(time_value.substr(kTimeMinuteOffset, 2)));
    if (kHour < 0 || kHour >= kHoursPerDay || kMinute < 0 ||
        kMinute >= kMinutesPerHour) {
      return std::nullopt;
    }
    return ((kHour * kMinutesPerHour) + kMinute) * kSecondsPerMinute;
  } catch (const std::exception&) {
    return std::nullopt;
  }
}

[[nodiscard]] auto CalculateDurationSeconds(const std::string& start_time_str,
                                            const std::string& end_time_str)
    -> int {
  if (start_time_str.length() != kTimeStringLength ||
      end_time_str.length() != kTimeStringLength) {
    return 0;
  }

  try {
    const int kStartHour =
        std::stoi(start_time_str.substr(kTimeHourOffset, kTimeHourLength));
    const int kStartMinute =
        std::stoi(start_time_str.substr(kTimeMinuteOffset, kTimeMinuteLength));
    const int kEndHour =
        std::stoi(end_time_str.substr(kTimeHourOffset, kTimeHourLength));
    const int kEndMinute =
        std::stoi(end_time_str.substr(kTimeMinuteOffset, kTimeMinuteLength));

    const int kStartSeconds =
        (kStartHour * kMinutesPerHour + kStartMinute) * kSecondsPerMinute;
    int end_seconds =
        (kEndHour * kMinutesPerHour + kEndMinute) * kSecondsPerMinute;
    if (end_seconds < kStartSeconds) {
      end_seconds += kSecondsPerDay;
    }
    return end_seconds - kStartSeconds;
  } catch (const std::exception&) {
    return 0;
  }
}

[[nodiscard]] auto TimeStringToTimestamp(const std::string& date,
                                         const std::string& time,
                                         bool is_end_time,
                                         long long start_timestamp_for_end)
    -> long long {
  if (date.length() != kDateStringLength ||
      time.length() != kTimeStringLength) {
    return 0;
  }

  long long timestamp = StringToTimeT(date + " " + time);
  if (is_end_time && timestamp < start_timestamp_for_end) {
    timestamp += static_cast<long long>(kSecondsPerDay);
  }
  return timestamp;
}

struct StatsRule {
  const char* match_path;
  int ActivityStats::* member;
};

constexpr std::array kStatsRules = {
    StatsRule{.match_path = "study", .member = &ActivityStats::study_time},
    StatsRule{.match_path = "exercise",
              .member = &ActivityStats::total_exercise_time},
    StatsRule{.match_path = "exercise_cardio",
              .member = &ActivityStats::cardio_time},
    StatsRule{.match_path = "exercise_anaerobic",
              .member = &ActivityStats::anaerobic_time},
    StatsRule{.match_path = "routine_grooming",
              .member = &ActivityStats::grooming_time},
    StatsRule{.match_path = "routine_toilet",
              .member = &ActivityStats::toilet_time},
    StatsRule{.match_path = "recreation_game",
              .member = &ActivityStats::gaming_time},
    StatsRule{.match_path = "recreation",
              .member = &ActivityStats::recreation_time},
    StatsRule{.match_path = "recreation_zhihu",
              .member = &ActivityStats::recreation_zhihu_time},
    StatsRule{.match_path = "recreation_bilibili",
              .member = &ActivityStats::recreation_bilibili_time},
    StatsRule{.match_path = "recreation_douyin",
              .member = &ActivityStats::recreation_douyin_time}};

}  // namespace

auto NormalizeTime(std::string_view time_str) -> std::string {
  if (time_str.length() == kTimeDigitsLength) {
    return std::string(time_str.substr(kTimeHourOffset, kTimeHourLength)) +
           ":" +
           std::string(
               time_str.substr(kTimeDigitsMinuteOffset, kTimeMinuteLength));
  }
  return std::string(time_str);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto CalculateWrappedDurationSeconds(std::string_view start_hhmm,
                                     std::string_view end_hhmm)
    -> std::optional<int> {
  const std::optional<int> kStartSeconds = ParseHhmmToSeconds(start_hhmm);
  const std::optional<int> kEndSeconds = ParseHhmmToSeconds(end_hhmm);
  if (!kStartSeconds.has_value() || !kEndSeconds.has_value()) {
    return std::nullopt;
  }

  int wrapped_end_seconds = *kEndSeconds;
  if (wrapped_end_seconds < *kStartSeconds) {
    wrapped_end_seconds += kSecondsPerDay;
  }
  return wrapped_end_seconds - *kStartSeconds;
}

auto MergeSpans(const std::optional<SourceSpan>& start_span,
                const std::optional<SourceSpan>& end_span)
    -> std::optional<SourceSpan> {
  if (start_span.has_value() && end_span.has_value()) {
    SourceSpan merged = *start_span;
    if (!end_span->file_path.empty()) {
      merged.file_path = end_span->file_path;
    }
    if (end_span->line_start > 0 &&
        (merged.line_start == 0 || end_span->line_start < merged.line_start)) {
      merged.line_start = end_span->line_start;
    }
    merged.line_end = std::max(end_span->line_end, merged.line_end);
    return merged;
  }
  if (start_span.has_value()) {
    return start_span;
  }
  if (end_span.has_value()) {
    return end_span;
  }
  return std::nullopt;
}

void DayStats::CalculateStats(DailyLog& day) {
  day.activityCount = static_cast<int>(day.processedActivities.size());
  day.stats = {};
  day.hasStudyActivity = false;
  day.hasExerciseActivity = false;

  long long activity_sequence = 1;
  long long date_as_long = 0;
  try {
    std::string temp_date = day.date;
    temp_date.erase(std::ranges::remove(temp_date, '-').begin(),
                    temp_date.end());
    date_as_long = std::stoll(temp_date);
  } catch (const std::invalid_argument&) {
    return;
  }

  for (auto& activity : day.processedActivities) {
    activity.logical_id =
        (date_as_long * kDailySequenceBase) + activity_sequence++;
    activity.duration_seconds = CalculateDurationSeconds(
        activity.start_time_str, activity.end_time_str);

    activity.start_timestamp =
        TimeStringToTimestamp(day.date, activity.start_time_str, false, 0);
    activity.end_timestamp = TimeStringToTimestamp(
        day.date, activity.end_time_str, true, activity.start_timestamp);

    if (activity.project_path.starts_with("study")) {
      day.hasStudyActivity = true;
    }
    if (activity.project_path.starts_with("exercise")) {
      day.hasExerciseActivity = true;
    }

    if (activity.project_path == "sleep_night") {
      day.stats.sleep_night_time += activity.duration_seconds;
    }
    if (activity.project_path == "sleep_day") {
      day.stats.sleep_day_time += activity.duration_seconds;
    }

    for (const auto& rule : kStatsRules) {
      if (activity.project_path.starts_with(rule.match_path)) {
        (day.stats.*(rule.member)) += activity.duration_seconds;
      }
    }
  }

  day.stats.sleep_total_time =
      day.stats.sleep_night_time + day.stats.sleep_day_time;
}

}  // namespace converter_core_internal
