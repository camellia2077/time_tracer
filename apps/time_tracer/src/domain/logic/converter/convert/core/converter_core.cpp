// domain/logic/converter/convert/core/converter_core.cpp
#include "domain/logic/converter/convert/core/converter_core.hpp"

#include <algorithm>
#include <array>
#include <ctime>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string_view>

#include "domain/ports/diagnostics.hpp"
#include "shared/utils/string_utils.hpp"

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

auto NormalizeTime(std::string_view time_str) -> std::string {
  if (time_str.length() == kTimeDigitsLength) {
    return std::string(time_str.substr(kTimeHourOffset, kTimeHourLength)) +
           ":" +
           std::string(
               time_str.substr(kTimeDigitsMinuteOffset, kTimeMinuteLength));
  }
  return std::string(time_str);
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

class DayStats {
 public:
  static void CalculateStats(DailyLog& day) {
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

 private:
  static auto CalculateDurationSeconds(const std::string& start_time_str,
                                       const std::string& end_time_str) -> int {
    if (start_time_str.length() != kTimeStringLength ||
        end_time_str.length() != kTimeStringLength) {
      return 0;
    }
    try {
      int start_hour =
          std::stoi(start_time_str.substr(kTimeHourOffset, kTimeHourLength));
      int start_min = std::stoi(
          start_time_str.substr(kTimeMinuteOffset, kTimeMinuteLength));
      int end_hour =
          std::stoi(end_time_str.substr(kTimeHourOffset, kTimeHourLength));
      int end_min =
          std::stoi(end_time_str.substr(kTimeMinuteOffset, kTimeMinuteLength));

      int start_seconds =
          (start_hour * kMinutesPerHour + start_min) * kSecondsPerMinute;
      int end_seconds =
          (end_hour * kMinutesPerHour + end_min) * kSecondsPerMinute;
      if (end_seconds < start_seconds) {
        end_seconds += kSecondsPerDay;
      }
      return end_seconds - start_seconds;
    } catch (const std::exception&) {
      return 0;
    }
  }

  static auto TimeStringToTimestamp(const std::string& date,
                                    const std::string& time, bool is_end_time,
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
};

class ActivityMapper {
 public:
  explicit ActivityMapper(const ConverterConfig& config)
      : config_(config), wake_keywords_(config.wake_keywords) {}

  auto MapActivities(DailyLog& day) -> void {
    day.processedActivities.clear();

    if (day.getupTime.empty() && !day.isContinuation) {
      return;
    }

    std::string start_time = day.getupTime;
    std::optional<SourceSpan> start_span;

    for (const auto& raw_event : day.rawEvents) {
      if (IsWakeEvent(raw_event)) {
        if (start_time.empty()) {
          start_time = NormalizeTime(raw_event.endTimeStr);
          start_span = raw_event.source_span;
        }
        continue;
      }

      std::string formatted_event_end_time =
          NormalizeTime(raw_event.endTimeStr);
      TimeRange range{.start_hhmm = start_time,
                      .end_hhmm = formatted_event_end_time};

      std::string mapped_description = MapDescription(raw_event.description);
      const int kDuration = CalculateDurationMinutes(range);
      mapped_description = ApplyDurationRules(mapped_description, kDuration);

      AppendActivity(day, raw_event, range, mapped_description, start_span);
      start_time = formatted_event_end_time;
      start_span = raw_event.source_span;
    }
  }

 private:
  struct TimeRange {
    std::string_view start_hhmm;
    std::string_view end_hhmm;
  };

  const ConverterConfig& config_;
  const std::vector<std::string>& wake_keywords_;

  [[nodiscard]] auto IsWakeEvent(const RawEvent& raw_event) const -> bool {
    return std::ranges::find(wake_keywords_, raw_event.description) !=
           wake_keywords_.end();
  }

  [[nodiscard]] auto MapDescription(std::string_view description) const
      -> std::string {
    std::string mapped_description(description);

    auto map_it = config_.text_mapping.find(mapped_description);
    if (map_it != config_.text_mapping.end()) {
      mapped_description = map_it->second;
    }

    auto duration_map_it =
        config_.text_duration_mapping.find(mapped_description);
    if (duration_map_it != config_.text_duration_mapping.end()) {
      mapped_description = duration_map_it->second;
    }
    return mapped_description;
  }

  [[nodiscard]] auto ApplyDurationRules(std::string_view mapped_description,
                                        int duration_minutes) const
      -> std::string {
    std::string description_str(mapped_description);
    auto duration_rules_it = config_.duration_mappings.find(description_str);
    if (duration_rules_it == config_.duration_mappings.end()) {
      return description_str;
    }

    for (const auto& rule : duration_rules_it->second) {
      if (duration_minutes < rule.less_than_minutes) {
        return rule.value;
      }
    }
    return description_str;
  }

  auto ApplyTopParentMapping(std::vector<std::string>& parts) const -> void {
    if (parts.empty()) {
      return;
    }

    auto map_it = config_.top_parent_mapping.find(parts.front());
    if (map_it != config_.top_parent_mapping.end()) {
      parts.front() = map_it->second;
      return;
    }

    auto init_map_it = config_.initial_top_parents.find(parts.front());
    if (init_map_it != config_.initial_top_parents.end()) {
      parts.front() = init_map_it->second;
    }
  }

  [[nodiscard]] static auto BuildProjectPath(
      const std::vector<std::string>& parts) -> std::string {
    std::stringstream path_stream;
    for (size_t i = 0; i < parts.size(); ++i) {
      path_stream << parts[i] << (i + 1 < parts.size() ? "_" : "");
    }
    return path_stream.str();
  }

  auto AppendActivity(DailyLog& day, const RawEvent& raw_event,
                      const TimeRange& time_range,
                      std::string_view mapped_description,
                      const std::optional<SourceSpan>& start_span) const
      -> void {
    if (time_range.start_hhmm.empty()) {
      return;
    }

    std::vector<std::string> parts =
        SplitString(std::string(mapped_description), '_');
    if (parts.empty()) {
      return;
    }

    BaseActivityRecord activity;
    activity.start_time_str = std::string(time_range.start_hhmm);
    activity.end_time_str = std::string(time_range.end_hhmm);

    ApplyTopParentMapping(parts);
    activity.project_path = BuildProjectPath(parts);
    if (!raw_event.remark.empty()) {
      activity.remark = raw_event.remark;
    }
    activity.source_span = MergeSpans(start_span, raw_event.source_span);

    day.processedActivities.push_back(std::move(activity));
  }

  [[nodiscard]] static auto CalculateDurationMinutes(const TimeRange& range)
      -> int {
    if (range.start_hhmm.length() != kTimeStringLength ||
        range.end_hhmm.length() != kTimeStringLength) {
      return 0;
    }
    try {
      int start_hour = std::stoi(std::string(
          range.start_hhmm.substr(kTimeHourOffset, kTimeHourLength)));
      int start_min = std::stoi(std::string(
          range.start_hhmm.substr(kTimeMinuteOffset, kTimeMinuteLength)));
      int end_hour = std::stoi(
          std::string(range.end_hhmm.substr(kTimeHourOffset, kTimeHourLength)));
      int end_min = std::stoi(std::string(
          range.end_hhmm.substr(kTimeMinuteOffset, kTimeMinuteLength)));

      int start_time_minutes = (start_hour * kMinutesPerHour) + start_min;
      int end_time_minutes = (end_hour * kMinutesPerHour) + end_min;
      if (end_time_minutes < start_time_minutes) {
        end_time_minutes += kHoursPerDay * kMinutesPerHour;
      }
      return end_time_minutes - start_time_minutes;
    } catch (const std::exception&) {
      return 0;
    }
  }
};

}  // namespace

DayProcessor::DayProcessor(const ConverterConfig& config) : config_(config) {}

void DayProcessor::Process(DailyLog& previous_day, DailyLog& day_to_process) {
  if (day_to_process.date.empty()) {
    return;
  }

  ActivityMapper activity_mapper(config_);
  activity_mapper.MapActivities(day_to_process);

  if (!previous_day.date.empty() && !previous_day.rawEvents.empty() &&
      !day_to_process.getupTime.empty() && !day_to_process.isContinuation) {
    BaseActivityRecord sleep_activity;
    sleep_activity.start_time_str =
        NormalizeTime(previous_day.rawEvents.back().endTimeStr);
    sleep_activity.end_time_str = day_to_process.getupTime;
    sleep_activity.project_path = "sleep_night";

    day_to_process.processedActivities.insert(
        day_to_process.processedActivities.begin(), sleep_activity);
    day_to_process.hasSleepActivity = true;
  }

  if (day_to_process.isContinuation && !previous_day.rawEvents.empty()) {
    day_to_process.getupTime =
        NormalizeTime(previous_day.rawEvents.back().endTimeStr);
  }

  DayStats::CalculateStats(day_to_process);
}

LogLinker::LogLinker(const ConverterConfig& config) : config_(config) {}

void LogLinker::LinkLogs(
    std::map<std::string, std::vector<DailyLog>>& data_map) {
  DailyLog* prev_month_last_day = nullptr;
  int linked_count = 0;

  for (auto& [month_key, days] : data_map) {
    static_cast<void>(month_key);
    if (days.empty()) {
      continue;
    }

    DailyLog& current_first_day = days.front();
    if (prev_month_last_day != nullptr) {
      const bool kHasValidGetup = !current_first_day.getupTime.empty() &&
                                  current_first_day.getupTime != "00:00";
      const bool kMissingSleep = !current_first_day.hasSleepActivity;
      if (kHasValidGetup && kMissingSleep) {
        ProcessCrossDay(current_first_day, *prev_month_last_day);
        linked_count++;
      }
    }
    prev_month_last_day = &days.back();
  }

  if (linked_count > 0) {
    time_tracer::domain::ports::EmitInfo("  [LogLinker] 已根据配置修复 " +
                                         std::to_string(linked_count) +
                                         " 处跨月睡眠记录。");
  }
}

void LogLinker::ProcessCrossDay(DailyLog& current_day,
                                const DailyLog& prev_day) {
  if (prev_day.rawEvents.empty()) {
    return;
  }

  BaseActivityRecord sleep_activity;
  sleep_activity.start_time_str =
      FormatTime(prev_day.rawEvents.back().endTimeStr);
  sleep_activity.end_time_str = current_day.getupTime;
  sleep_activity.project_path = config_.generated_sleep_project_path.empty()
                                    ? "sleep_night"
                                    : config_.generated_sleep_project_path;

  current_day.processedActivities.insert(
      current_day.processedActivities.begin(), sleep_activity);
  current_day.hasSleepActivity = true;
  RecalculateStats(current_day);
}

void LogLinker::RecalculateStats(DailyLog& day) {
  DayStats::CalculateStats(day);
}

auto LogLinker::FormatTime(std::string_view time_str) -> std::string {
  return NormalizeTime(time_str);
}
