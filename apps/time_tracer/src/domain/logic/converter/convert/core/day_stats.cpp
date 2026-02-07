// domain/logic/converter/convert/core/day_stats.cpp
#include "domain/logic/converter/convert/core/day_stats.hpp"

#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "domain/logic/converter/convert/core/stats_rules.hpp"

namespace {
// logical_id layout: (YYYYMMDD) as high bits, daily sequence as low bits.
// Example: date 20210101, sequence 42 -> 20210101000042.
constexpr long long kDailySequenceBase = 1000000;
constexpr size_t kDateTimeMinLength = 16;
constexpr size_t kDateStringLength = 10;
constexpr size_t kTimeStringLength = 5;
constexpr size_t kTimeHourOffset = 0;
constexpr size_t kTimeHourLength = 2;
constexpr size_t kTimeMinuteOffset = 3;
constexpr size_t kTimeMinuteLength = 2;
constexpr int kHoursPerDay = 24;
constexpr int kMinutesPerHour = 60;
constexpr int kSecondsPerMinute = 60;
constexpr int kSecondsPerDay =
    kHoursPerDay * kMinutesPerHour * kSecondsPerMinute;

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
}  // namespace

auto DayStats::CalculateDurationSeconds(const std::string& start_time_str,
                                        const std::string& end_time_str)
    -> int {
  if (start_time_str.length() != kTimeStringLength ||
      end_time_str.length() != kTimeStringLength) {
    return 0;
  }
  try {
    int start_hour =
        std::stoi(start_time_str.substr(kTimeHourOffset, kTimeHourLength));
    int start_min =
        std::stoi(start_time_str.substr(kTimeMinuteOffset, kTimeMinuteLength));
    int end_hour =
        std::stoi(end_time_str.substr(kTimeHourOffset, kTimeHourLength));
    int end_min =
        std::stoi(end_time_str.substr(kTimeMinuteOffset, kTimeMinuteLength));
    int start_time_in_seconds =
        (start_hour * kMinutesPerHour + start_min) * kSecondsPerMinute;
    int end_time_in_seconds =
        (end_hour * kMinutesPerHour + end_min) * kSecondsPerMinute;
    if (end_time_in_seconds < start_time_in_seconds) {
      end_time_in_seconds += kSecondsPerDay;
    }
    return end_time_in_seconds - start_time_in_seconds;
  } catch (const std::exception&) {
    return 0;
  }
}

auto DayStats::TimeStringToTimestamp(const std::string& date,
                                     const std::string& time, bool is_end_time,
                                     long long start_timestamp_for_end)
    -> long long {
  if (date.length() != kDateStringLength ||
      time.length() != kTimeStringLength) {
    return 0;
  }

  std::string datetime_str = date + " " + time;
  long long timestamp = StringToTimeT(datetime_str);

  if (is_end_time && timestamp < start_timestamp_for_end) {
    timestamp += static_cast<long long>(kSecondsPerDay);
  }
  return timestamp;
}

void DayStats::CalculateStats(DailyLog& day) {
  day.activityCount = static_cast<int>(day.processedActivities.size());
  day.stats = {};  // [适配] generatedStats -> stats
  day.hasStudyActivity = false;
  day.hasExerciseActivity = false;

  long long activity_sequence = 1;
  long long date_as_long = 0;
  try {
    std::string temp_date = day.date;
    auto [first, end] = std::ranges::remove(temp_date, '-');
    temp_date.erase(first, end);
    date_as_long = std::stoll(temp_date);
  } catch (const std::invalid_argument&) {
    return;
  }

  for (auto& activity : day.processedActivities) {
    activity.logical_id =
        (date_as_long * kDailySequenceBase) + activity_sequence++;
    // [适配] startTime/endTime -> start_time_str/end_time_str
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

    // [适配] 字段名 generatedStats -> stats，以及驼峰转 snake_case
    if (activity.project_path == "sleep_night") {
      day.stats.sleep_night_time += activity.duration_seconds;
    }
    if (activity.project_path == "sleep_day") {
      day.stats.sleep_day_time += activity.duration_seconds;
    }

    // 使用规则反射进行统计
    for (const auto& rule : StatsRules::kRules) {
      if (activity.project_path.starts_with(rule.match_path)) {
        (day.stats.*(rule.member)) += activity.duration_seconds;
      }
    }
  }

  day.stats.sleep_total_time =
      day.stats.sleep_night_time + day.stats.sleep_day_time;
}
