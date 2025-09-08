// reprocessing/converter/internal/Converter.cpp
#include "Converter.hpp"
#include <stdexcept>
#include <unordered_set>
#include "common/common_utils.hpp" // For split_string

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
    : config_(config),
      wake_keywords_(config.getWakeKeywords().begin(), config.getWakeKeywords().end()) {}

void Converter::transform(InputData& day) {
    day.processedActivities.clear();
    
    if (day.getupTime.empty() && !day.isContinuation) return;
    
    std::string startTime = day.getupTime;

    for (const auto& rawEvent : day.rawEvents) {
        if (wake_keywords_.count(rawEvent.description)) {
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
            std::vector<std::string> parts = split_string(mappedDescription, '_');
            if (!parts.empty()) {
                Activity activity;
                activity.startTime = startTime;
                activity.endTime = formattedEventEndTime;
                
                // [核心修改] 使用新的成员变量和 Getter
                activity.top_parent = parts[0];
                const auto& top_parents_map = config_.getTopParentMapping();
                auto map_it = top_parents_map.find(activity.top_parent);
                if (map_it != top_parents_map.end()) {
                    activity.top_parent = map_it->second;
                }

                if (parts.size() > 1) {
                    activity.parents.assign(parts.begin() + 1, parts.end());
                }
                day.processedActivities.push_back(activity);
            }
        }
        startTime = formattedEventEndTime;
    }
}