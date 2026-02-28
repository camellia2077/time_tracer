// domain/logic/converter/convert/core/converter_core.cpp
#include "domain/logic/converter/convert/core/converter_core.hpp"

#include <optional>
#include <string>
#include <string_view>

#include "domain/logic/converter/convert/core/converter_core_internal.hpp"
#include "domain/ports/diagnostics.hpp"

namespace {

constexpr int kMaxLinkedSleepDurationSeconds = 16 * 60 * 60;

}  // namespace

DayProcessor::DayProcessor(const ConverterConfig& config) : config_(config) {}

void DayProcessor::Process(DailyLog& previous_day, DailyLog& day_to_process) {
  if (day_to_process.date.empty()) {
    return;
  }

  if (day_to_process.isContinuation && !previous_day.rawEvents.empty()) {
    day_to_process.getupTime = converter_core_internal::NormalizeTime(
        previous_day.rawEvents.back().endTimeStr);
  }

  converter_core_internal::ActivityMapper activity_mapper(config_);
  activity_mapper.MapActivities(day_to_process);

  if (!previous_day.date.empty() && !previous_day.rawEvents.empty() &&
      !day_to_process.getupTime.empty() && !day_to_process.isContinuation) {
    BaseActivityRecord sleep_activity;
    sleep_activity.start_time_str = converter_core_internal::NormalizeTime(
        previous_day.rawEvents.back().endTimeStr);
    sleep_activity.end_time_str = day_to_process.getupTime;
    sleep_activity.project_path = "sleep_night";

    day_to_process.processedActivities.insert(
        day_to_process.processedActivities.begin(), sleep_activity);
    day_to_process.hasSleepActivity = true;
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
    tracer_core::domain::ports::EmitInfo("  [LogLinker] 已根据配置修复 " +
                                         std::to_string(linked_count) +
                                         " 处跨月睡眠记录。");
  }
}

void LogLinker::LinkFirstDayWithExternalPreviousEvent(
    std::map<std::string, std::vector<DailyLog>>& data_map,
    std::string_view previous_date, std::string_view previous_end_time) {
  if (data_map.empty()) {
    return;
  }

  auto first_month_iter = data_map.begin();
  if (first_month_iter->second.empty()) {
    return;
  }

  DailyLog& current_first_day = first_month_iter->second.front();
  const bool has_valid_getup = !current_first_day.getupTime.empty() &&
                               current_first_day.getupTime != "00:00";
  const bool missing_sleep = !current_first_day.hasSleepActivity;
  if (!has_valid_getup || !missing_sleep) {
    return;
  }

  DailyLog previous_day;
  previous_day.date = std::string(previous_date);
  previous_day.rawEvents.push_back(
      RawEvent{.endTimeStr = std::string(previous_end_time)});
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

  BaseActivityRecord sleep_activity;
  sleep_activity.start_time_str = kStartTime;
  sleep_activity.end_time_str = kEndTime;
  sleep_activity.project_path = config_.generated_sleep_project_path.empty()
                                    ? "sleep_night"
                                    : config_.generated_sleep_project_path;

  current_day.processedActivities.insert(
      current_day.processedActivities.begin(), sleep_activity);
  current_day.hasSleepActivity = true;
  RecalculateStats(current_day);
}

void LogLinker::RecalculateStats(DailyLog& day) {
  converter_core_internal::DayStats::CalculateStats(day);
}

auto LogLinker::FormatTime(std::string_view time_str) -> std::string {
  return converter_core_internal::NormalizeTime(time_str);
}
