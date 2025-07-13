#include "IntervalConverter.h"
#include <stdexcept>

// 辅助函数
namespace {
    std::string formatTime(const std::string& timeStrHHMM) {
        if (timeStrHHMM.length() == 4) {
            return timeStrHHMM.substr(0, 2) + ":" + timeStrHHMM.substr(2, 2);
        }
        return timeStrHHMM;
    }

    int calculateDurationMinutes(const std::string& startTimeStr, const std::string& endTimeStr) {
        if (startTimeStr.length() != 5 || endTimeStr.length() != 5) return 0;
        try {
            int startHour = std::stoi(startTimeStr.substr(0, 2));
            int startMin = std::stoi(startTimeStr.substr(3, 2));
            int endHour = std::stoi(endTimeStr.substr(0, 2));
            int endMin = std::stoi(endTimeStr.substr(3, 2));
            int startTimeInMinutes = startHour * 60 + startMin;
            int endTimeInMinutes = endHour * 60 + endMin;
            if (endTimeInMinutes < startTimeInMinutes) {
                endTimeInMinutes += 24 * 60;
            }
            return endTimeInMinutes - startTimeInMinutes;
        } catch (const std::exception&) {
            return 0;
        }
    }
}

IntervalConverter::IntervalConverter(const IntervalProcessorConfig& config)
    : config_(config) {}

void IntervalConverter::transform(InputData& day) {
    day.remarksOutput.clear();
    for (const auto& general_remark : day.generalRemarks) {
        day.remarksOutput.push_back(general_remark);
    }
    
    if (day.getupTime.empty() && !day.isContinuation) return;
    
    std::string startTime = day.getupTime;
    for (const auto& rawEvent : day.rawEvents) {
        if (rawEvent.description == "起床" || rawEvent.description == "醒") {
            if (startTime.empty()) {
                 startTime = formatTime(rawEvent.endTimeStr);
            }
            continue;
        }

        std::string formattedEventEndTime = formatTime(rawEvent.endTimeStr);
        std::string mappedDescription = rawEvent.description;
        
        auto mapIt = config_.getTextMapping().find(mappedDescription);
        if (mapIt != config_.getTextMapping().end()) {
            mappedDescription = mapIt->second;
        }

        auto durMapIt = config_.getTextDurationMapping().find(mappedDescription);
        if (durMapIt != config_.getTextDurationMapping().end()) {
            mappedDescription = durMapIt->second;
        }
        
        auto durationRulesIt = config_.getDurationMappings().find(mappedDescription);
        if (durationRulesIt != config_.getDurationMappings().end()) {
            int duration = calculateDurationMinutes(startTime, formattedEventEndTime);
            for (const auto& rule : durationRulesIt->second) {
                if (duration < rule.less_than_minutes) {
                    mappedDescription = rule.value; 
                    break; 
                }
            }
        }

        if (mappedDescription.find("study") != std::string::npos) day.hasStudyActivity = true;
        if (!startTime.empty()) {
            day.remarksOutput.push_back(startTime + "~" + formattedEventEndTime + mappedDescription);
        }
        startTime = formattedEventEndTime;
    }
}