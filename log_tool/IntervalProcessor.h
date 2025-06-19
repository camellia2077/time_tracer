// IntervalProcessor.h

#ifndef INTERVAL_PROCESSOR_H
#define INTERVAL_PROCESSOR_H

#include <string>
#include <vector>
#include <unordered_map>
#include "json.hpp" // Assumes nlohmann/json.hpp is available

class IntervalProcessor {
public:
    IntervalProcessor(const std::string& config_filename, const std::string& header_config_filename); // 修改：接收新的配置文件
    bool processFile(const std::string& input_filepath, const std::string& output_filepath);

private:
    // ... (RawEvent 和 DayData 结构体保持不变)
    struct RawEvent {
        std::string endTimeStr;
        std::string description;
    };

    struct DayData {
        std::string date;
        bool hasStudyActivity = false;
        std::string getupTime;
        std::vector<RawEvent> rawEvents;
        std::vector<std::string> remarksOutput;
        void clear();
    };

    // --- Configuration and state ---
    std::string config_filepath_;
    std::string header_config_filepath_; // 新增
    std::unordered_map<std::string, std::string> text_mapping_;
    std::vector<std::string> header_order_; // 新增

    // --- Private helper methods ---
    bool loadConfiguration(); // 修改：合并两个配置加载
    void processDayData(DayData& day);
    void writeDayData(std::ofstream& outFile, const DayData& day);
    std::string formatTime(const std::string& timeStrHHMM);
    bool isDateLine(const std::string& line);
    bool parseEventLine(const std::string& line, std::string& outTimeStr, std::string& outDescription);
};

#endif // INTERVAL_PROCESSOR_H