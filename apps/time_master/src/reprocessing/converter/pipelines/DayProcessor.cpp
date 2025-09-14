// reprocessing/converter/pipelines/DayProcessor.cpp
#include "DayProcessor.hpp"
#include "DayStatsCalculator.hpp"
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
        sleepActivity.endTime = dayToProcess.getupTime; // Fixed typo here
        sleepActivity.topParent = "sleep";
        sleepActivity.parents = {"night"};

        dayToProcess.processedActivities.insert(dayToProcess.processedActivities.begin(), sleepActivity);
        dayToProcess.hasSleepActivity = true;
    }

    if (dayToProcess.isContinuation && !previousDay.rawEvents.empty()) {
        dayToProcess.getupTime = formatTime(previousDay.rawEvents.back().endTimeStr);
    }
    
    // [修改] DayStatsCalculator 不再需要 config
    DayStatsCalculator stats_calculator;
    stats_calculator.calculate_stats(dayToProcess);
}