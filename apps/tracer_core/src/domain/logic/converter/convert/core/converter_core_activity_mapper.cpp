// domain/logic/converter/convert/core/converter_core_activity_mapper.cpp
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "domain/logic/converter/convert/core/converter_core_internal.hpp"
#include "shared/utils/string_utils.hpp"

namespace converter_core_internal {
namespace {

constexpr int kHoursPerDay = 24;
constexpr int kMinutesPerHour = 60;
constexpr size_t kTimeStringLength = 5;
constexpr size_t kTimeHourOffset = 0;
constexpr size_t kTimeHourLength = 2;
constexpr size_t kTimeMinuteOffset = 3;
constexpr size_t kTimeMinuteLength = 2;

}  // namespace

ActivityMapper::ActivityMapper(const ConverterConfig& config)
    : config_(config), wake_keywords_(config.wake_keywords) {}

auto ActivityMapper::MapActivities(DailyLog& day) -> void {
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

    std::string formatted_event_end_time = NormalizeTime(raw_event.endTimeStr);
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

[[nodiscard]] auto ActivityMapper::IsWakeEvent(const RawEvent& raw_event) const
    -> bool {
  return std::ranges::find(wake_keywords_, raw_event.description) !=
         wake_keywords_.end();
}

[[nodiscard]] auto ActivityMapper::MapDescription(
    std::string_view description) const -> std::string {
  std::string mapped_description(description);

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

[[nodiscard]] auto ActivityMapper::ApplyDurationRules(
    std::string_view mapped_description, int duration_minutes) const
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

auto ActivityMapper::ApplyTopParentMapping(
    std::vector<std::string>& parts) const -> void {
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

[[nodiscard]] auto ActivityMapper::BuildProjectPath(
    const std::vector<std::string>& parts) -> std::string {
  std::stringstream path_stream;
  for (size_t i = 0; i < parts.size(); ++i) {
    path_stream << parts[i] << (i + 1 < parts.size() ? "_" : "");
  }
  return path_stream.str();
}

auto ActivityMapper::AppendActivity(
    DailyLog& day, const RawEvent& raw_event, const TimeRange& time_range,
    std::string_view mapped_description,
    const std::optional<SourceSpan>& start_span) const -> void {
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

[[nodiscard]] auto ActivityMapper::CalculateDurationMinutes(
    const TimeRange& range) -> int {
  if (range.start_hhmm.length() != kTimeStringLength ||
      range.end_hhmm.length() != kTimeStringLength) {
    return 0;
  }
  try {
    const int kStartHour = std::stoi(
        std::string(range.start_hhmm.substr(kTimeHourOffset, kTimeHourLength)));
    const int kStartMinute = std::stoi(std::string(
        range.start_hhmm.substr(kTimeMinuteOffset, kTimeMinuteLength)));
    const int kEndHour = std::stoi(
        std::string(range.end_hhmm.substr(kTimeHourOffset, kTimeHourLength)));
    const int kEndMinute = std::stoi(std::string(
        range.end_hhmm.substr(kTimeMinuteOffset, kTimeMinuteLength)));

    const int kStartTimeMinutes = (kStartHour * kMinutesPerHour) + kStartMinute;
    int end_time_minutes = (kEndHour * kMinutesPerHour) + kEndMinute;
    if (end_time_minutes < kStartTimeMinutes) {
      end_time_minutes += kHoursPerDay * kMinutesPerHour;
    }
    return end_time_minutes - kStartTimeMinutes;
  } catch (const std::exception&) {
    return 0;
  }
}

}  // namespace converter_core_internal
