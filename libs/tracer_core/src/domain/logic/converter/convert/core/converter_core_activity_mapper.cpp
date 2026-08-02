// domain/logic/converter/convert/core/converter_core_activity_mapper.cpp
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "domain/logic/converter/convert/core/converter_core_internal.hpp"

import tracer.core.shared.string_utils;

namespace converter_core_internal {

using tracer::core::shared::string_utils::SplitString;

namespace {

constexpr int kHoursPerDay = 24;
constexpr int kMinutesPerHour = 60;
constexpr int kSecondsPerMinute = 60;
constexpr size_t kTimeStringLength = 8;
constexpr size_t kTimeHourOffset = 0;
constexpr size_t kTimeHourLength = 2;
constexpr size_t kTimeMinuteOffset = 3;
constexpr size_t kTimeMinuteLength = 2;
constexpr size_t kTimeSecondOffset = 6;
constexpr size_t kTimeSecondLength = 2;

[[nodiscard]] auto IsValidExplicitIntervalClockRange(
    std::string_view start_hhmm, std::string_view end_hhmm) -> bool {
  if (start_hhmm.length() != kTimeStringLength ||
      end_hhmm.length() != kTimeStringLength || start_hhmm[2] != ':' ||
      start_hhmm[5] != ':' || end_hhmm[2] != ':' || end_hhmm[5] != ':') {
    return false;
  }

  try {
    const int start_hour = std::stoi(
        std::string(start_hhmm.substr(kTimeHourOffset, kTimeHourLength)));
    const int start_minute = std::stoi(
        std::string(start_hhmm.substr(kTimeMinuteOffset, kTimeMinuteLength)));
    const int end_hour = std::stoi(
        std::string(end_hhmm.substr(kTimeHourOffset, kTimeHourLength)));
    const int end_minute = std::stoi(
        std::string(end_hhmm.substr(kTimeMinuteOffset, kTimeMinuteLength)));
    const int start_second = std::stoi(
        std::string(start_hhmm.substr(kTimeSecondOffset, kTimeSecondLength)));
    const int end_second = std::stoi(
        std::string(end_hhmm.substr(kTimeSecondOffset, kTimeSecondLength)));
    const int start_total =
        ((start_hour * kMinutesPerHour) + start_minute) * kSecondsPerMinute +
        start_second;
    const int end_total =
        ((end_hour * kMinutesPerHour) + end_minute) * kSecondsPerMinute +
        end_second;
    if (start_total == end_total) {
      return false;
    }
    return true;
  } catch (const std::exception&) {
    return false;
  }
}

[[nodiscard]] auto HasExplicitIntervalEvent(const DailyLog& day) -> bool {
  return std::ranges::any_of(day.rawEvents, [](const RawEvent& raw_event) {
    return raw_event.kind == RawEventKind::Interval;
  });
}

}  // namespace

ActivityMapper::ActivityMapper(const ConverterConfig& config)
    : config_(config), wake_keywords_(config.sleep_inference.wake_keywords) {}

auto ActivityMapper::MapActivities(DailyLog& day) -> void {
  day.processedActivities.clear();

  if (day.getupTime.empty() && !day.isContinuation &&
      !HasExplicitIntervalEvent(day)) {
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
    std::string explicit_interval_start;
    std::optional<SourceSpan> effective_start_span = start_span;
    TimeRange range{.start_hhmm = start_time,
                    .end_hhmm = formatted_event_end_time};
    if (raw_event.kind == RawEventKind::Interval) {
      // Point events derive [last_known_boundary, end). Interval events keep
      // their authored [start, end) range, and the explicit interval end still
      // becomes the boundary for the next point event.
      if (!raw_event.startTimeStr.has_value()) {
        continue;
      }
      explicit_interval_start = NormalizeTime(*raw_event.startTimeStr);
      if (!IsValidExplicitIntervalClockRange(explicit_interval_start,
                                             formatted_event_end_time)) {
        continue;
      }
      range.start_hhmm = explicit_interval_start;
      effective_start_span = raw_event.source_span;
    }

    std::string mapped_description = MapDescription(raw_event.description);
    AppendActivity(day, raw_event, range, mapped_description,
                   effective_start_span);
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

  // Canonical mapping stage:
  // TXT stores alias keys (raw activity tokens). During full-text conversion/ingest we map
  // alias key -> canonical value expanded from the fixed activity hierarchy
  // TOML directory.
  // Timing semantics are applied later: alias normalization decides only the
  // canonical activity path, while time ranges and durations are derived from
  // neighboring authored event timestamps in subsequent steps.
  auto map_it = config_.text_mapping.find(mapped_description);
  if (map_it != config_.text_mapping.end()) {
    mapped_description = map_it->second;
  }

  return mapped_description;
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
  std::vector<std::string> parts =
      SplitString(std::string(mapped_description), '_');
  if (parts.empty()) {
    return;
  }

  ApplyTopParentMapping(parts);
  const std::string kProjectPath = BuildProjectPath(parts);
  BaseActivityRecord activity =
      time_range.start_hhmm.empty()
          ? BaseActivityRecord::MakeEndOnly(std::string(time_range.end_hhmm),
                                            kProjectPath)
          : BaseActivityRecord::MakeInterval(
                std::string(time_range.start_hhmm),
                std::string(time_range.end_hhmm), kProjectPath);
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
