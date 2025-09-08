// reprocessing/converter/internal/converter/ActivityMapper.cpp
#include "ActivityMapper.hpp"
#include "common/common_utils.hpp" // For split_string
#include <stdexcept>

// [修改] 将辅助函数定义为类的成员函数
std::string ActivityMapper::formatTime(const std::string& timeStrHHMM) const {
    if (timeStrHHMM.length() == 4) {
        return timeStrHHMM.substr(0, 2) + ":" + timeStrHHMM.substr(2, 2);
    }
    return timeStrHHMM;
}

// [修改] 将辅助函数定义为类的成员函数
int ActivityMapper::calculateDurationMinutes(const std::string& startTimeStr, const std::string& endTimeStr) const {
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

ActivityMapper::ActivityMapper(const ConverterConfig& config)
    : config_(config),
      wake_keywords_(config.getWakeKeywords().begin(), config.getWakeKeywords().end()) {}

void ActivityMapper::map_activities(InputData& day) {
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
        
        if (!startTime.empty()) {
            std::vector<std::string> parts = split_string(mappedDescription, '_');
            if (!parts.empty()) {
                Activity activity;
                activity.startTime = startTime;
                activity.endTime = formattedEventEndTime;
                
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