// reprocessing/converter/model/InputData.hpp
#ifndef INPUT_DATA_HPP
#define INPUT_DATA_HPP

#include <string>
#include <vector>
#include <optional>

struct RawEvent {
    std::string endTimeStr;
    std::string description;
    std::string remark; // 新增：用于存储原始备注信息
};

struct Activity {
    //逻辑ID，用于报告展示排序
    long long logical_id; // e.g., 202501010001, 202501010002...
    // 方案二：Unix时间戳，用于精确计算和分析
    long long start_timestamp; // e.g., 1735689600
    long long end_timestamp;   // e.g., 1735693200
    std::string startTime;
    std::string endTime;
    std::string topParent;
    std::vector<std::string> parents;
    int durationSeconds = 0;
    std::optional<std::string> activityRemark; // 用于存储活动的备注// 新增：用于存储活动的备注
};

struct GeneratedStats {
    int sleepTime = 0;
    // --- [核心修改] 将 exerciseTime 重命名为 totalExerciseTime ---
    int totalExerciseTime = 0;
    // --- [核心修改] 新增不同运动类型的统计字段 ---
    int cardioTime = 0;
    int anaerobicTime = 0;
    int exerciseBothTime = 0;
};

struct InputData {
    std::string date;
    bool hasStudyActivity = false;
    bool hasExerciseActivity = false; // 新增：标记当天是否有运动
    bool hasSleepActivity = false; // 新增：标记当天是否包含睡眠活动

    std::string getupTime;
    std::vector<std::string> generalRemarks;
    std::vector<RawEvent> rawEvents;
    
    std::vector<Activity> processedActivities;

    bool isContinuation = false;

    int activityCount = 0;
    GeneratedStats generatedStats; 

    void clear() {
        date.clear();
        hasStudyActivity = false;
        hasExerciseActivity = false; // 新增
        hasSleepActivity = false; // [核心修改]
        getupTime.clear();
        generalRemarks.clear();
        rawEvents.clear();
        processedActivities.clear();
        isContinuation = false;
        activityCount = 0;
        generatedStats = {};
    }
};

#endif // INPUT_DATA_HPP