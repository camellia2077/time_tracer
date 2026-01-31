// converter/convert/core/day_stats.cpp
#include "day_stats.hpp"

#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "stats_rules.hpp"

namespace {
auto StringToTimeT(const std::string& datetime_str) -> long long {
  if (datetime_str.length() < 16) {
    return 0;
  }
  std::tm t = {};
  std::stringstream ss(datetime_str);
  ss >> std::get_time(&t, "%Y-%m-%d %H:%M");

  if (ss.fail()) {
    return 0;
  }
  return std::mktime(&t);
}
}  // namespace

int DayStats::calculateDurationSeconds(const std::string& startTimeStr,
                                       const std::string& endTimeStr) {
  if (startTimeStr.length() != 5 || endTimeStr.length() != 5) {
    return 0;
  }
  try {
    int start_hour = std::stoi(startTimeStr.substr(0, 2));
    int start_min = std::stoi(startTimeStr.substr(3, 2));
    int end_hour = std::stoi(endTimeStr.substr(0, 2));
    int end_min = std::stoi(endTimeStr.substr(3, 2));
    int start_time_in_seconds = (start_hour * 60 + start_min) * 60;
    int end_time_in_seconds = (end_hour * 60 + end_min) * 60;
    if (end_time_in_seconds < start_time_in_seconds) {
      end_time_in_seconds += 24 * 60 * 60;
    }
    return end_time_in_seconds - start_time_in_seconds;
  } catch (const std::exception&) {
    return 0;
  }
}

long long DayStats::timeStringToTimestamp(const std::string& date,
                                          const std::string& time,
                                          bool is_end_time,
                                          long long start_timestamp_for_end) {
  if (date.length() != 10 || time.length() != 5) {
    return 0;
  }

  std::string datetime_str = date + " " + time;
  long long timestamp = StringToTimeT(datetime_str);

  if (is_end_time && timestamp < start_timestamp_for_end) {
    timestamp += 24 * 60 * 60;
  }
  return timestamp;
}

void DayStats::calculate_stats(DailyLog& day) {
  day.activityCount = day.processedActivities.size();
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
    activity.logical_id = (date_as_long * 10000) + activity_sequence++;
    // [适配] startTime/endTime -> start_time_str/end_time_str
    activity.duration_seconds = calculateDurationSeconds(
        activity.start_time_str, activity.end_time_str);

    activity.start_timestamp =
        timeStringToTimestamp(day.date, activity.start_time_str, false, 0);
    activity.end_timestamp = timeStringToTimestamp(
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
    for (const auto& rule : StatsRules::rules) {
      if (activity.project_path.starts_with(rule.match_path)) {
        (day.stats.*(rule.member)) += activity.duration_seconds;
      }
    }
  }

  day.stats.sleep_total_time =
      day.stats.sleep_night_time + day.stats.sleep_day_time;
}