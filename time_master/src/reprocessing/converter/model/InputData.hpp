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
    std::string title;
    std::vector<std::string> parents;
};

// 代表一天所有日志数据的结构体
struct InputData {
    std::string date;
    bool hasStudyActivity = false;
    bool endsWithSleepNight = false;
    std::string getupTime;
    std::vector<std::string> generalRemarks;
    std::vector<RawEvent> rawEvents;
    
    // [修改] 用新的结构化向量取代原来的字符串向量
    // std::vector<std::string> remarksOutput;
    std::vector<Activity> processedActivities;

    bool isContinuation = false;

    void clear() {
        date.clear();
        hasStudyActivity = false;
        endsWithSleepNight = false;
        getupTime.clear();
        generalRemarks.clear();
        rawEvents.clear();
        processedActivities.clear();
        isContinuation = false;
    }
};

#endif // INPUT_DATA_HPP