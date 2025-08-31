// converter/internal/DayProcessor.cpp
#include "DayProcessor.hpp"

namespace {
    std::string formatTime(const std::string& timeStrHHMM) {
        return (timeStrHHMM.length() == 4) ? timeStrHHMM.substr(0, 2) + ":" + timeStrHHMM.substr(2, 2) : timeStrHHMM;
    }
}

DayProcessor::DayProcessor(Converter& converter) : converter_(converter) {}

void DayProcessor::process(InputData& dayToFinalize, InputData& nextDay) {
    if (dayToFinalize.date.empty()) return;
    
    converter_.transform(dayToFinalize);

    if (nextDay.isContinuation) {
        dayToFinalize.endsWithSleepNight = false;
        if (!dayToFinalize.rawEvents.empty()) {
            nextDay.getupTime = formatTime(dayToFinalize.rawEvents.back().endTimeStr);
        }
    } else if (!dayToFinalize.isContinuation && !nextDay.getupTime.empty()) {
        if (!dayToFinalize.rawEvents.empty()) {
            std::string lastEventTime = formatTime(dayToFinalize.rawEvents.back().endTimeStr);
            
            // [修改] 添加结构化的 sleep_night 活动，而不是拼接字符串
            Activity sleepActivity;
            sleepActivity.startTime = lastEventTime;
            sleepActivity.endTime = nextDay.getupTime;
            sleepActivity.title = "sleep";
            sleepActivity.parents = {"night"};
            dayToFinalize.processedActivities.push_back(sleepActivity);

            dayToFinalize.endsWithSleepNight = true;
        }
    }
}