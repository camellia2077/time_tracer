// domain/logic/converter/convert/core/log_linker.cpp
#include "domain/logic/converter/convert/core/log_linker.hpp"

#include <algorithm>
#include <iostream>

#include "domain/logic/converter/convert/core/day_stats.hpp"
#include "shared/types/ansi_colors.hpp"

LogLinker::LogLinker(const ConverterConfig& config) : config_(config) {}

void LogLinker::LinkLogs(
    std::map<std::string, std::vector<DailyLog>>& data_map) {
  std::cout << "[Debug] LogLinker::LinkLogs start. Map size: "
            << data_map.size() << std::endl;

  DailyLog* prev_month_last_day = nullptr;
  int linked_count = 0;

  for (auto& [month_key, days] : data_map) {
    if (days.empty()) {
      continue;
    }

    DailyLog& current_first_day = days.front();

    // 调试日志 (可选保留)
    // std::cout << "[Debug] Checking Month: " << month_key << ", First Day: "
    // << current_first_day.date << std::endl;

    if (prev_month_last_day != nullptr) {
      // [规则修正] 严格遵守 "无起床关键词即通宵" 规则
      // 只要有明确的起床时间，且还没生成睡眠活动，就应该尝试链接。
      bool has_valid_getup = !current_first_day.getupTime.empty() &&
                             current_first_day.getupTime != "00:00";
      bool missing_sleep = !current_first_day.hasSleepActivity;

      if (has_valid_getup && missing_sleep) {
        // std::cout << "  -> [MATCH] Linking triggered!" << std::endl;
        ProcessCrossDay(current_first_day, *prev_month_last_day);
        linked_count++;
      }
    }

    prev_month_last_day = &days.back();
  }

  if (linked_count > 0) {
    namespace colors = time_tracer::common::colors;
    std::cout << colors::kGreen << "  [LogLinker] 已根据配置修复 "
              << linked_count << " 处跨月睡眠记录。" << colors::kReset
              << std::endl;
  }
}

void LogLinker::ProcessCrossDay(DailyLog& current_day,
                                const DailyLog& prev_day) {
  if (prev_day.rawEvents.empty()) {
    return;
  }

  // 1. 获取上个月最后时刻 (作为入睡时间)
  std::string last_end_time_raw = prev_day.rawEvents.back().endTimeStr;
  std::string sleep_start_time = FormatTime(last_end_time_raw);

  // 2. 获取当前月起床时刻 (作为醒来时间)
  std::string sleep_end_time = current_day.getupTime;

  // 3. 构造睡眠活动
  BaseActivityRecord sleep_activity;
  sleep_activity.start_time_str = sleep_start_time;
  sleep_activity.end_time_str = sleep_end_time;

  // 使用配置注入的项目路径，移除硬编码 "sleep_night"
  // 如果配置为空（通常 Loader 会保证默认值），则降级使用默认值
  sleep_activity.project_path = config_.generated_sleep_project_path.empty()
                                    ? "sleep_night"
                                    : config_.generated_sleep_project_path;
  // 4. 插入记录并更新状态
  current_day.processedActivities.insert(
      current_day.processedActivities.begin(), sleep_activity);
  current_day.hasSleepActivity = true;

  // 5. 重新计算统计数据 (刷新时长)
  RecalculateStats(current_day);
}

void LogLinker::RecalculateStats(DailyLog& day) {
  DayStats::CalculateStats(day);
}

auto LogLinker::FormatTime(const std::string& timeStr) -> std::string {
  if (timeStr.length() == 4) {
    return timeStr.substr(0, 2) + ":" + timeStr.substr(2, 2);
  }
  return timeStr;
}
