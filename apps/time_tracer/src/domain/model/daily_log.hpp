// domain/model/daily_log.hpp
#ifndef DOMAIN_MODEL_DAILY_LOG_H_
#define DOMAIN_MODEL_DAILY_LOG_H_

#include <optional>
#include <string>
#include <vector>

#include "domain/model/source_span.hpp"
#include "domain/model/time_data_models.hpp"

// RawEvent 属于 Converter 解析阶段的中间产物，保留在此
struct RawEvent {
  std::string endTimeStr;
  std::string description;
  std::string remark;
  std::optional<SourceSpan> source_span;
};

// [核心修改] 移除 Activity 和 GeneratedStats 的定义

struct DailyLog {
  std::string date;
  bool hasStudyActivity = false;
  bool hasExerciseActivity = false;
  bool hasSleepActivity = false;

  std::string getupTime;
  std::vector<std::string> generalRemarks;
  std::vector<RawEvent> rawEvents;
  std::optional<SourceSpan> source_span;

  // [核心修改] 使用 BaseActivityRecord
  std::vector<BaseActivityRecord> processedActivities;

  bool isContinuation = false;

  int activityCount = 0;

  // [核心修改] 使用 ActivityStats，并重命名为 stats
  ActivityStats stats;

  void Clear() {
    date.clear();
    hasStudyActivity = false;
    hasExerciseActivity = false;
    hasSleepActivity = false;
    getupTime.clear();
    generalRemarks.clear();
    rawEvents.clear();
    source_span.reset();
    processedActivities.clear();
    isContinuation = false;
    activityCount = 0;
    stats = {};  // 重置为默认值
  }
};

#endif  // DOMAIN_MODEL_DAILY_LOG_H_
