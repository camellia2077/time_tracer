// reprocessing/converter/model/InputData.h
#ifndef INPUT_DATA_HPP
#define INPUT_DATA_HPP

#include <string>
#include <vector>

// 原始事件的最小表示
struct RawEvent {
    std::string endTimeStr;
    std::string description;
};

// 代表一天所有日志数据的结构体
struct InputData {
    std::string date;
    bool hasStudyActivity = false;
    bool endsWithSleepNight = false;
    std::string getupTime;
    std::vector<std::string> generalRemarks;
    std::vector<RawEvent> rawEvents;
    std::vector<std::string> remarksOutput;
    bool isContinuation = false;

    void clear() {
        date.clear();
        hasStudyActivity = false;
        endsWithSleepNight = false;
        getupTime.clear();
        generalRemarks.clear();
        rawEvents.clear();
        remarksOutput.clear();
        isContinuation = false;
    }
};

#endif // INPUT_DATA_HPP