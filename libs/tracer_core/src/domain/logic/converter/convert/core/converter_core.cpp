// domain/logic/converter/convert/core/converter_core.cpp
#include "domain/logic/converter/convert/core/converter_core.hpp"

#include <optional>
#include <string>
#include <string_view>

#include "domain/logic/converter/convert/core/converter_core_internal.hpp"
#include "domain/ports/diagnostics.hpp"

namespace {

constexpr int kMaxLinkedSleepDurationSeconds = 16 * 60 * 60;

auto ResolveSleepInferenceProjectPath(const ConverterConfig& config)
    -> std::string {
  // This only controls the generated overnight sleep activity path.
  return config.sleep_inference.sleep_project_path.empty()
             ? "sleep_night"
             : config.sleep_inference.sleep_project_path;
}

auto HasInferredOvernightSleep(const DailyLog& day,
                               const ConverterConfig& config) -> bool {
  // Avoid inserting the same generated overnight sleep activity twice during
  // linking.
  return !day.processedActivities.empty() &&
         (day.processedActivities.front().project_path ==
          ResolveSleepInferenceProjectPath(config));
}

auto ResolveMappedDescription(const RawEvent& raw_event,
                              const ConverterConfig& config) -> std::string {
  const auto kMapping = config.text_mapping.find(raw_event.description);
  return kMapping == config.text_mapping.end() ? raw_event.description
                                              : kMapping->second;
}

auto HasExplicitSleepInterval(const DailyLog& day,
                              const ConverterConfig& config) -> bool {
  for (const auto& raw_event : day.rawEvents) {
    if (raw_event.kind != RawEventKind::Interval) {
      continue;
    }

    const std::string kMapped = ResolveMappedDescription(raw_event, config);
    if (kMapped == config.sleep_inference.sleep_project_path ||
        kMapped.starts_with("sleep_") || kMapped.starts_with("sleep/")) {
      return true;
    }
  }
  return false;
}

auto ShouldUsePreviousBoundaryForLeadingPoint(const DailyLog& day) -> bool {
  return day.isContinuation && !day.rawEvents.empty() &&
         day.rawEvents.front().kind == RawEventKind::Point;
}

}  // namespace

DayProcessor::DayProcessor(const ConverterConfig& config) : config_(config) {}

void DayProcessor::Process(DailyLog& previous_day, DailyLog& day_to_process) {
  if (day_to_process.date.empty()) {
    return;
  }

  if (ShouldUsePreviousBoundaryForLeadingPoint(day_to_process) &&
      !previous_day.rawEvents.empty()) {
    day_to_process.getupTime = converter_core_internal::NormalizeTime(
        previous_day.rawEvents.back().endTimeStr);
  }

  converter_core_internal::ActivityMapper activity_mapper(config_);
  activity_mapper.MapActivities(day_to_process);

  // If the day starts with a valid getup time, synthesize an overnight sleep
  // activity from the previous day's tail.
  if (!previous_day.date.empty() && !previous_day.rawEvents.empty() &&
      !day_to_process.getupTime.empty() && !day_to_process.isContinuation &&
      !HasExplicitSleepInterval(previous_day, config_)) {
    BaseActivityRecord sleep_activity = BaseActivityRecord::MakeInterval(
        converter_core_internal::NormalizeTime(
            previous_day.rawEvents.back().endTimeStr),
        day_to_process.getupTime, ResolveSleepInferenceProjectPath(config_));

    day_to_process.processedActivities.insert(
        day_to_process.processedActivities.begin(), sleep_activity);
  }

  converter_core_internal::DayStats::CalculateStats(day_to_process);
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
      const bool kHasValidGetup =
          !current_first_day.getupTime.empty() &&
          converter_core_internal::NormalizeTime(current_first_day.getupTime) !=
              "00:00:00";
      const bool kMissingInferredSleep =
          !HasInferredOvernightSleep(current_first_day, config_);
      // Linking depends on a valid getup time and whether overnight sleep was
      // already synthesized, not on generic sleep_* activity presence.
      if (kHasValidGetup && kMissingInferredSleep) {
        ProcessCrossDay(current_first_day, *prev_month_last_day);
        linked_count++;
      }
    }
    prev_month_last_day = &days.back();
  }

  if (linked_count > 0) {
    tracer_core::domain::ports::EmitInfo("  [LogLinker] 已根据配置修复 " +
                                         std::to_string(linked_count) +
                                         " 处跨月睡眠记录。");
  }
}

void LogLinker::LinkFirstDayWithExternalPreviousEvent(
    std::map<std::string, std::vector<DailyLog>>& data_map,
    const ExternalPreviousEvent& previous_event) {
  if (data_map.empty()) {
    return;
  }

  auto first_month_iter = data_map.begin();
  if (first_month_iter->second.empty()) {
    return;
  }

  DailyLog& current_first_day = first_month_iter->second.front();
  const bool kHasValidGetup = !current_first_day.getupTime.empty() &&
                              converter_core_internal::NormalizeTime(
                                  current_first_day.getupTime) != "00:00:00";
  const bool kMissingInferredSleep =
      !HasInferredOvernightSleep(current_first_day, config_);
  if (!kHasValidGetup || !kMissingInferredSleep) {
    return;
  }

  DailyLog previous_day;
  previous_day.date = std::string(previous_event.date);
  previous_day.rawEvents.push_back(
      RawEvent{.endTimeStr = std::string(previous_event.end_time)});
  ProcessCrossDay(current_first_day, previous_day);
}

void LogLinker::ProcessCrossDay(DailyLog& current_day,
                                const DailyLog& prev_day) {
  if (prev_day.rawEvents.empty()) {
    return;
  }

  const std::string kStartTime =
      FormatTime(prev_day.rawEvents.back().endTimeStr);
  const std::string kEndTime = current_day.getupTime;
  const std::optional<int> kDurationSeconds =
      converter_core_internal::CalculateWrappedDurationSeconds(kStartTime,
                                                               kEndTime);
  if (!kDurationSeconds.has_value()) {
    return;
  }
  if (*kDurationSeconds > kMaxLinkedSleepDurationSeconds) {
    tracer_core::domain::ports::EmitWarn(
        "[LogLinker] Skip cross-month sleep link for " + current_day.date +
        ": duration would exceed 16 hours (start=" + kStartTime +
        ", end=" + kEndTime + ").");
    return;
  }

  BaseActivityRecord sleep_activity = BaseActivityRecord::MakeInterval(
      kStartTime, kEndTime, ResolveSleepInferenceProjectPath(config_));

  // Insert the generated overnight sleep activity into the fact set.
  current_day.processedActivities.insert(
      current_day.processedActivities.begin(), sleep_activity);
  RecalculateStats(current_day);
}

void LogLinker::RecalculateStats(DailyLog& day) {
  converter_core_internal::DayStats::CalculateStats(day);
}

auto LogLinker::FormatTime(std::string_view time_str) -> std::string {
  return converter_core_internal::NormalizeTime(time_str);
}
