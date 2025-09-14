// reprocessing/converter/model/InputData.hpp
#ifndef INPUT_DATA_HPP
#define INPUT_DATA_HPP

#include <string>
#include <vector>
#include <optional>

struct RawEvent {
    std::string endTimeStr;
    std::string description;
    std::string remark; 
};

struct Activity {
    long long logical_id; 
    long long start_timestamp; 
    long long end_timestamp;   
    std::string startTime;
    std::string endTime;
    std::string topParent;
    std::vector<std::string> parents;
    int durationSeconds = 0;
    std::optional<std::string> activityRemark;
};

// [核心修改] 改回使用固定的成员变量
struct GeneratedStats {
    int sleepTime = 0;
    int totalExerciseTime = 0;
    int cardioTime = 0;
    int anaerobicTime = 0;
    int groomingTime = 0;
    int toiletTime = 0;
    int gamingTime = 0;
};

struct InputData {
    std::string date;
    bool hasStudyActivity = false;
    bool hasExerciseActivity = false; 
    bool hasSleepActivity = false; 

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
        hasExerciseActivity = false;
        hasSleepActivity = false;
        getupTime.clear();
        generalRemarks.clear();
        rawEvents.clear();
        processedActivities.clear();
        isContinuation = false;
        activityCount = 0;
        generatedStats = {}; // 重置为默认值
    }
};

#endif // INPUT_DATA_HPP