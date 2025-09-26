// reprocessing/converter/pipelines/DayProcessor.cpp
#include "DayProcessor.hpp"
#include "DayStats.hpp"
#include "ActivityMapper.hpp" 

namespace {
    std::string formatTime(const std::string& timeStrHHMM) {
        return (timeStrHHMM.length() == 4) ? timeStrHHMM.substr(0, 2) + ":" + timeStrHHMM.substr(2, 2) : timeStrHHMM;
    }
}

DayProcessor::DayProcessor(const ConverterConfig& config) : config_(config) {}

void DayProcessor::process(InputData& previousDay, InputData& dayToProcess) {
    if (dayToProcess.date.empty()) return;

    ActivityMapper activity_mapper(config_);
    activity_mapper.map_activities(dayToProcess);

    if (!previousDay.date.empty() && !previousDay.rawEvents.empty() && !dayToProcess.getupTime.empty() && !dayToProcess.isContinuation) {
        std::string lastEventTime = formatTime(previousDay.rawEvents.back().endTimeStr);

        Activity sleepActivity;
        sleepActivity.startTime = lastEventTime;
        sleepActivity.endTime = dayToProcess.getupTime;
        
        // --- [CORE FIX] ---
        // The struct 'Activity' no longer has 'parent' or 'children' members.
        // Assign the combined path to the 'project_path' member instead.
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