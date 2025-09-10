// reprocessing/converter/model/InputData.hpp
#ifndef INPUT_DATA_HPP
#define INPUT_DATA_HPP

#include <string>
#include <vector>
#include <optional>

struct RawEvent {
    std::string endTimeStr;
    std::string description;
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
};

struct GeneratedStats {
    int sleepTime = 0;
};

struct InputData {
    std::string date;
    bool hasStudyActivity = false;
    bool endsWithSleepNight = false;
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
        endsWithSleepNight = false;
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