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

// [优化] 在构造函数的初始化列表中创建哈希表
Converter::Converter(const ConverterConfig& config)
    : config_(config),
      wake_keywords_(config.getWakeKeywords().begin(), config.getWakeKeywords().end()) {}

void Converter::transform(InputData& day) {
    day.processedActivities.clear(); // 清空，准备填充新的结构化数据
    
    if (day.getupTime.empty() && !day.isContinuation) return;
    
    std::string startTime = day.getupTime;

    for (const auto& rawEvent : day.rawEvents) {
        // [优化] 直接使用成员变量 wake_keywords_
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
            // [核心修改] 将映射后的描述符（如 game_steam_fps_overwatch）拆分为 title 和 parents
            std::vector<std::string> parts = split_string(mappedDescription, '_');
            if (!parts.empty()) {
                Activity activity;
                activity.startTime = startTime;
                activity.endTime = formattedEventEndTime;
                
                // [核心修改] 应用 initial_top_parents 映射
                activity.title = parts[0]; // 1. 获取原始 title
                const auto& top_parents_map = config_.getInitialTopParentsMapping();
                auto map_it = top_parents_map.find(activity.title);
                if (map_it != top_parents_map.end()) {
                    activity.title = map_it->second; // 2. 如果在映射中找到，则替换为新值
                }

                if (parts.size() > 1) {
                    // 剩余的元素是 parents 列表
                    activity.parents.assign(parts.begin() + 1, parts.end());
                }
                day.processedActivities.push_back(activity);
            }
        }
        startTime = formattedEventEndTime;
    }
}