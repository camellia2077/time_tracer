// converter/convert/core/day_processor.cpp
#include "day_processor.hpp"
#include "day_stats.hpp"
#include "activity_mapper.hpp" 

namespace {
    std::string formatTime(const std::string& timeStrHHMM) {
        return (timeStrHHMM.length() == 4) ? timeStrHHMM.substr(0, 2) + ":" + timeStrHHMM.substr(2, 2) : timeStrHHMM;
    }
}

DayProcessor::DayProcessor(const ConverterConfig& config) : config_(config) {}

void DayProcessor::process(DailyLog& previousDay, DailyLog& dayToProcess) {
    if (dayToProcess.date.empty()) return;

    ActivityMapper activity_mapper(config_);
    activity_mapper.map_activities(dayToProcess);

    if (!previousDay.date.empty() && !previousDay.rawEvents.empty() && !dayToProcess.getupTime.empty() && !dayToProcess.isContinuation) {
        std::string lastEventTime = formatTime(previousDay.rawEvents.back().endTimeStr);

        // [核心修改] 使用 BaseActivityRecord
        BaseActivityRecord sleepActivity;
        // [适配] 字段名变更
        sleepActivity.start_time_str = lastEventTime;
        sleepActivity.end_time_str = dayToProcess.getupTime;
        sleepActivity.project_path = "sleep_night";

        dayToProcess.processedActivities.insert(dayToProcess.processedActivities.begin(), sleepActivity);
        dayToProcess.hasSleepActivity = true;
    }

    if (dayToProcess.isContinuation && !previousDay.rawEvents.empty()) {
        dayToProcess.getupTime = formatTime(previousDay.rawEvents.back().endTimeStr);
    }
    
    DayStats stats_calculator;
    stats_calculator.calculate_stats(dayToProcess);
}