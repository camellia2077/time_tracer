// reprocessing/input_transfer/internal/Converter.cpp
#include "Converter.h"
#include <stdexcept>
#include <unordered_set> // [修改] 1. 引入 unordered_set 头文件

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

Converter::Converter(const ConverterConfig& config)
    : config_(config) {}

void Converter::transform(InputData& day) {
    day.remarksOutput.clear();
    for (const auto& general_remark : day.generalRemarks) {
        day.remarksOutput.push_back(general_remark);
    }
    
    if (day.getupTime.empty() && !day.isContinuation) return;
    
    std::string startTime = day.getupTime;

    // [修改] 2. 定义包含所有起床关键词的集合
    const std::unordered_set<std::string> wake_keywords = {"起床", "醒", "w", "wake"};

    for (const auto& rawEvent : day.rawEvents) {
        // [修改] 3. 使用集合的 .count() 方法进行判断，替代原来的多个 "||"
        if (wake_keywords.count(rawEvent.description)) {
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