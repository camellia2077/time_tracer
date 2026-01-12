// common/model/DailyLog.hpp
#ifndef DAILY_LOG_HPP
#define DAILY_LOG_HPP

#include <string>
#include <vector>
#include <optional>
#include "TimeDataModels.hpp" // 引入公共模型

// RawEvent 属于 Converter 解析阶段的中间产物，保留在此
struct RawEvent {
    std::string endTimeStr;
    std::string description;
    std::string remark; 
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
    
    // [核心修改] 使用 BaseActivityRecord
    std::vector<BaseActivityRecord> processedActivities;

    bool isContinuation = false;

    int activityCount = 0;
    
    // [核心修改] 使用 ActivityStats，并重命名为 stats
    ActivityStats stats; 

    void clear() {
        date.clear();
        hasStudyActivity = false;
        hasExerciseActivity = false;
        hasSleepActivity = false;
        getupTime.clear();
        generalRemarks.clear();
        rawEvents.clear();
        processedActivities.clear();
        isContinuation = false;
        activityCount = 0;
        stats = {}; // 重置为默认值
    }
};

#endif // DAILY_LOG_HPP