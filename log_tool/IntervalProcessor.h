// IntervalProcessor.h

#ifndef INTERVAL_PROCESSOR_H
#define INTERVAL_PROCESSOR_H

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

class IntervalProcessor {
public:
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
        bool endsWithSleepNight = false;
        std::string getupTime;
        std::vector<RawEvent> rawEvents;
        std::vector<std::string> remarksOutput;
        bool isContinuation = false; 
        
        void clear();
    };
    
    struct DurationRule {
        int less_than_minutes;
        std::string value;
    };

    std::string config_filepath_;
    std::string header_config_filepath_;
    std::unordered_map<std::string, std::string> text_mapping_;
    // --- MODIFICATION START ---
    std::unordered_map<std::string, std::string> text_duration_mapping_; // 新增：用于需要根据时长判断的活动的映射
    // --- MODIFICATION END ---
    std::vector<std::string> header_order_;
    std::unordered_map<std::string, std::vector<DurationRule>> duration_mappings_;

    bool loadConfiguration();
    void writeDayData(std::ofstream& outFile, const DayData& day);
    std::string formatTime(const std::string& timeStrHHMM);
    bool isDateLine(const std::string& line);
    bool parseEventLine(const std::string& line, std::string& outTimeStr, std::string& outDescription);
    int calculateDurationMinutes(const std::string& startTimeHHMM, const std::string& endTimeHHMM);
};

#endif // INTERVAL_PROCESSOR_H