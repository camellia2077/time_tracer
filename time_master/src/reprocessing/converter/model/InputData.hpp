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
    std::string startTime;
    std::string endTime;
    std::string top_parent;
    std::vector<std::string> parents;
    int durationSeconds = 0;
};

// [新增] 用于存储计算后的统计数据的结构体
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
    // [修改] 移除 sleepTime 字段，并新增 GeneratedStats
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
        generatedStats = {}; // 清理时重置
    }
};

#endif // INPUT_DATA_HPP