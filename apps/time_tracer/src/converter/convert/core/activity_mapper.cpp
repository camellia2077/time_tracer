// converter/convert/core/activity_mapper.cpp
#include "activity_mapper.hpp"

#include <algorithm>  // for std::find
#include <sstream>
#include <stdexcept>

#include "common/utils/string_utils.hpp"

std::string ActivityMapper::formatTime(const std::string& timeStrHHMM) {
  if (timeStrHHMM.length() == 4) {
    return timeStrHHMM.substr(0, 2) + ":" + timeStrHHMM.substr(2, 2);
  }
  return timeStrHHMM;
}

int ActivityMapper::calculateDurationMinutes(const std::string& startTimeStr,
                                             const std::string& endTimeStr) {
  if (startTimeStr.length() != 5 || endTimeStr.length() != 5) {
    return 0;
  }
  try {
    int start_hour = std::stoi(startTimeStr.substr(0, 2));
    int start_min = std::stoi(startTimeStr.substr(3, 2));
    int end_hour = std::stoi(endTimeStr.substr(0, 2));
    int end_min = std::stoi(endTimeStr.substr(3, 2));
    int start_time_in_minutes = (start_hour * 60) + start_min;
    int end_time_in_minutes = (end_hour * 60) + end_min;
    if (end_time_in_minutes < start_time_in_minutes) {
      end_time_in_minutes += 24 * 60;
    }
    return end_time_in_minutes - start_time_in_minutes;
  } catch (const std::exception&) {
    return 0;
  }
}

ActivityMapper::ActivityMapper(const ConverterConfig& config)
    : config_(config),
      wake_keywords_(config.wake_keywords)  // 直接绑定引用
{}

void ActivityMapper::map_activities(DailyLog& day) {
  day.processedActivities.clear();

  if (day.getupTime.empty() && !day.isContinuation) {
    return;
  }

  std::string start_time = day.getupTime;

  for (const auto& raw_event : day.rawEvents) {
    // [修复] vector 没有 count 方法，使用 std::find
    bool is_wake = std::ranges::find(wake_keywords_, raw_event.description) !=
                   wake_keywords_.end();

    if (is_wake) {
      if (start_time.empty()) {
        start_time = formatTime(raw_event.endTimeStr);
      }
      continue;
    }

    std::string formatted_event_end_time = formatTime(raw_event.endTimeStr);
    std::string mapped_description = raw_event.description;

    // [修复] 直接访问 public 成员
    auto map_it = config_.text_mapping.find(mapped_description);
    if (map_it != config_.text_mapping.end()) {
      mapped_description = map_it->second;
    }

    auto dur_map_it = config_.text_duration_mapping.find(mapped_description);
    if (dur_map_it != config_.text_duration_mapping.end()) {
      mapped_description = dur_map_it->second;
    }

    auto duration_rules_it = config_.duration_mappings.find(mapped_description);
    if (duration_rules_it != config_.duration_mappings.end()) {
      int duration =
          calculateDurationMinutes(start_time, formatted_event_end_time);
      for (const auto& rule : duration_rules_it->second) {
        if (duration < rule.less_than_minutes) {
          mapped_description = rule.value;
          break;
        }
      }
    }

    if (!start_time.empty()) {
      std::vector<std::string> parts = split_string(mapped_description, '_');
      if (!parts.empty()) {
        BaseActivityRecord activity;
        activity.start_time_str = start_time;
        activity.end_time_str = formatted_event_end_time;

        // [修复] 直接访问 public 成员
        const auto& top_parents_map = config_.top_parent_mapping;
        auto map_it = top_parents_map.find(parts[0]);
        if (map_it != top_parents_map.end()) {
          parts[0] = map_it->second;
        } else {
          // 检查运行时注入的配置 (Initial Top Parents)
          auto init_map_it = config_.initial_top_parents.find(parts[0]);
          if (init_map_it != config_.initial_top_parents.end()) {
            parts[0] = init_map_it->second;
          }
        }

        std::stringstream ss;
        for (size_t i = 0; i < parts.size(); ++i) {
          ss << parts[i] << (i < parts.size() - 1 ? "_" : "");
        }
        activity.project_path = ss.str();

        if (!raw_event.remark.empty()) {
          activity.remark = raw_event.remark;
        }

        day.processedActivities.push_back(activity);
      }
    }
    start_time = formatted_event_end_time;
  }
}