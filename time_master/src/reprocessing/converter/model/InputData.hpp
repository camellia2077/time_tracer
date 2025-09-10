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
    std::string topParent; // [核心修改] 将 top_parent 改为 topParent
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