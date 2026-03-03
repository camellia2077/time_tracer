// IntervalProcessor.h

#ifndef INTERVAL_PROCESSOR_H
#define INTERVAL_PROCESSOR_H

#include <string>
#include <vector>
#include <unordered_map>
#include "json.hpp" // Assumes nlohmann/json.hpp is available

class IntervalProcessor {
public:
    // 构造函数现在接收两个配置文件路径
    IntervalProcessor(const std::string& config_filename, const std::string& header_config_filename);
    bool processFile(const std::string& input_filepath, const std::string& output_filepath);

private:
    struct RawEvent {
        std::string endTimeStr;
        std::string description;
    };

    struct DayData {
        std::string date;
        bool hasStudyActivity = false;
        bool endsWithSleepNight = false; // 新增：标记最后一个活动是否为 sleep_night
        std::string getupTime;
        std::vector<RawEvent> rawEvents;
        std::vector<std::string> remarksOutput;

        void clear();
    };

    // --- Configuration and state ---
    std::string config_filepath_;
    std::string header_config_filepath_;
    std::unordered_map<std::string, std::string> text_mapping_;
    std::vector<std::string> header_order_;

    // --- Private helper methods ---
    bool loadConfiguration();
    void processDayData(DayData& day);
    void writeDayData(std::ofstream& outFile, const DayData& day);
    std::string formatTime(const std::string& timeStrHHMM);
    bool isDateLine(const std::string& line);
    bool parseEventLine(const std::string& line, std::string& outTimeStr, std::string& outDescription);
};

#endif // INTERVAL_PROCESSOR_H