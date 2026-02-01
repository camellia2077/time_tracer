// converter/convert/core/day_processor.cpp
#include "day_processor.hpp"

#include "activity_mapper.hpp"
#include "day_stats.hpp"

namespace {
auto FormatTime(const std::string& timeStrHHMM) -> std::string {
  return (timeStrHHMM.length() == 4)
             ? timeStrHHMM.substr(0, 2) + ":" + timeStrHHMM.substr(2, 2)
             : timeStrHHMM;
}
}  // namespace

DayProcessor::DayProcessor(const ConverterConfig& config) : config_(config) {}

void DayProcessor::process(DailyLog& previousDay, DailyLog& dayToProcess) {
  if (dayToProcess.date.empty()) {
    return;
  }

  ActivityMapper activity_mapper(config_);
  activity_mapper.map_activities(dayToProcess);

  if (!previousDay.date.empty() && !previousDay.rawEvents.empty() &&
      !dayToProcess.getupTime.empty() && !dayToProcess.isContinuation) {
    std::string last_event_time =
        FormatTime(previousDay.rawEvents.back().endTimeStr);

    // [核心修改] 使用 BaseActivityRecord
    BaseActivityRecord sleep_activity;
    // [适配] 字段名变更
    sleep_activity.start_time_str = last_event_time;
    sleep_activity.end_time_str = dayToProcess.getupTime;
    sleep_activity.project_path = "sleep_night";

    dayToProcess.processedActivities.insert(
        dayToProcess.processedActivities.begin(), sleep_activity);
    dayToProcess.hasSleepActivity = true;
  }

  if (dayToProcess.isContinuation && !previousDay.rawEvents.empty()) {
    dayToProcess.getupTime =
        FormatTime(previousDay.rawEvents.back().endTimeStr);
  }

  DayStats::calculate_stats(dayToProcess);
}
