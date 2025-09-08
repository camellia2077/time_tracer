// reprocessing/converter/model/InputData.hpp
#ifndef INPUT_DATA_HPP
#define INPUT_DATA_HPP

#include <string>
#include <vector>

// 原始事件的最小表示
struct RawEvent {
    std::string endTimeStr;
    std::string description;
};

// [新增] 用于存储结构化活动数据的结构体
struct Activity {
    std::string startTime;
    std::string endTime;
    std::string top_parent;
    std::vector<std::string> parents;
    int durationSeconds = 0;// [新增] 用于存储活动时长（以秒为单位）
};

// 代表一天所有日志数据的结构体
struct InputData {
    std::string date;
    bool hasStudyActivity = false;
    bool endsWithSleepNight = false;
    std::string getupTime;
    std::vector<std::string> generalRemarks;
    std::vector<RawEvent> rawEvents;
    
    std::vector<Activity> processedActivities;

    bool isContinuation = false;

    int activityCount = 0; // 计算字段：每日活动总数

    void clear() {
        date.clear();
        hasStudyActivity = false;
        endsWithSleepNight = false;
        getupTime.clear();
        generalRemarks.clear();
        rawEvents.clear();
        processedActivities.clear();
        isContinuation = false;
        activityCount = 0; // 清理时重置
    }
};

#endif // INPUT_DATA_HPP