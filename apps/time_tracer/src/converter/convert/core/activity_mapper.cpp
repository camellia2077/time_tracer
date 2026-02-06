// converter/convert/core/activity_mapper.cpp
#include "converter/convert/core/activity_mapper.hpp"

#include <algorithm>  // for std::find
#include <sstream>
#include <stdexcept>

#include "common/utils/string_utils.hpp"

namespace {
constexpr size_t kTimeDigitsLength = 4;
constexpr size_t kTimeStringLength = 5;
constexpr size_t kTimeHourOffset = 0;
constexpr size_t kTimeHourLength = 2;
constexpr size_t kTimeDigitsMinuteOffset = 2;
constexpr size_t kTimeMinuteOffset = 3;
constexpr size_t kTimeMinuteLength = 2;
constexpr int kMinutesPerHour = 60;
constexpr int kHoursPerDay = 24;
}  // namespace

auto ActivityMapper::formatTime(std::string_view time_str_hhmm) -> std::string {
  // Example: "0614" -> "06:14".
  if (time_str_hhmm.length() == kTimeDigitsLength) {
    return std::string(time_str_hhmm.substr(kTimeHourOffset, kTimeHourLength)) +
           ":" +
           std::string(time_str_hhmm.substr(kTimeDigitsMinuteOffset,
                                            kTimeMinuteLength));
  }
  return std::string(time_str_hhmm);
}

auto ActivityMapper::calculateDurationMinutes(const TimeRange& time_range)
    -> int {
  // Example: "06:14" -> minutes offset 6*60 + 14.
  if (time_range.start_hhmm.length() != kTimeStringLength ||
      time_range.end_hhmm.length() != kTimeStringLength) {
    return 0;
  }
  try {
    int start_hour = std::stoi(
        std::string(time_range.start_hhmm.substr(kTimeHourOffset, kTimeHourLength)));
    int start_min = std::stoi(std::string(
        time_range.start_hhmm.substr(kTimeMinuteOffset, kTimeMinuteLength)));
    int end_hour = std::stoi(
        std::string(time_range.end_hhmm.substr(kTimeHourOffset, kTimeHourLength)));
    int end_min = std::stoi(std::string(
        time_range.end_hhmm.substr(kTimeMinuteOffset, kTimeMinuteLength)));
    int start_time_in_minutes = (start_hour * kMinutesPerHour) + start_min;
    int end_time_in_minutes = (end_hour * kMinutesPerHour) + end_min;
    if (end_time_in_minutes < start_time_in_minutes) {
      end_time_in_minutes += kHoursPerDay * kMinutesPerHour;
    }
    return end_time_in_minutes - start_time_in_minutes;
  } catch (const std::exception&) {
    return 0;
  }
}

ActivityMapper::ActivityMapper(const ConverterConfig& config)
    : config_(config),
      wake_keywords_(config.wake_keywords)  // 直接绑定引用
{}

auto ActivityMapper::is_wake_event(const RawEvent& raw_event) const -> bool {
  return std::ranges::find(wake_keywords_, raw_event.description) !=
         wake_keywords_.end();
}

auto ActivityMapper::map_description(std::string_view description) const
    -> std::string {
  std::string description_str(description);
  std::string mapped_description = description_str;

  auto map_it = config_.text_mapping.find(mapped_description);
  if (map_it != config_.text_mapping.end()) {
    mapped_description = map_it->second;
  }

  auto duration_map_it = config_.text_duration_mapping.find(mapped_description);
  if (duration_map_it != config_.text_duration_mapping.end()) {
    mapped_description = duration_map_it->second;
  }

  return mapped_description;
}

auto ActivityMapper::apply_duration_rules(std::string_view mapped_description,
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

void ActivityMapper::apply_top_parent_mapping(
    std::vector<std::string>& parts) const {
  if (parts.empty()) {
    return;
  }

  const auto& top_parents_map = config_.top_parent_mapping;
  auto map_it = top_parents_map.find(parts.front());
  if (map_it != top_parents_map.end()) {
    parts.front() = map_it->second;
    return;
  }

  // 检查运行时注入的配置 (Initial Top Parents)
  auto init_map_it = config_.initial_top_parents.find(parts.front());
  if (init_map_it != config_.initial_top_parents.end()) {
    parts.front() = init_map_it->second;
  }
}

auto ActivityMapper::build_project_path(const std::vector<std::string>& parts)
    -> std::string {
  std::stringstream path_stream;
  for (size_t i = 0; i < parts.size(); ++i) {
    path_stream << parts[i] << (i + 1 < parts.size() ? "_" : "");
  }
  return path_stream.str();
}

void ActivityMapper::append_activity(
    DailyLog& day, const RawEvent& raw_event, const TimeRange& time_range,
    std::string_view mapped_description) const {
  if (time_range.start_hhmm.empty()) {
    return;
  }

  std::string description_str(mapped_description);
  std::vector<std::string> parts = SplitString(description_str, '_');
  if (parts.empty()) {
    return;
  }

  BaseActivityRecord activity;
  activity.start_time_str = time_range.start_hhmm;
  activity.end_time_str = time_range.end_hhmm;

  apply_top_parent_mapping(parts);
  activity.project_path = build_project_path(parts);

  if (!raw_event.remark.empty()) {
    activity.remark = raw_event.remark;
  }

  day.processedActivities.push_back(activity);
}

void ActivityMapper::map_activities(DailyLog& day) {
  day.processedActivities.clear();



  if (day.getupTime.empty() && !day.isContinuation) {
    return;
  }

  std::string start_time = day.getupTime;

  for (const auto& raw_event : day.rawEvents) {
    bool is_wake = is_wake_event(raw_event);

    if (is_wake) {
      if (start_time.empty()) {
        start_time = formatTime(raw_event.endTimeStr);
      }
      continue;
    }

    std::string formatted_event_end_time = formatTime(raw_event.endTimeStr);
    TimeRange range{.start_hhmm = start_time,
                    .end_hhmm = formatted_event_end_time};

    std::string mapped_description = map_description(raw_event.description);
    int duration = calculateDurationMinutes(range);

    mapped_description = apply_duration_rules(mapped_description, duration);
    append_activity(day, raw_event, range, mapped_description);
    start_time = formatted_event_end_time;
  }
}
