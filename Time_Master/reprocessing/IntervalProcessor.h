// --- START OF FILE IntervalProcessor.h --- (MODIFIED)

#ifndef INTERVAL_PROCESSOR_H
#define INTERVAL_PROCESSOR_H

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <fstream> 
#include <utility>

class IntervalProcessor {
public:
    IntervalProcessor(const std::string& config_filename);

    bool executeConversion(const std::string& input_filepath, const std::string& output_filepath, const std::string& year_prefix);

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
        std::vector<std::string> generalRemarks;
        std::vector<RawEvent> rawEvents;
        std::vector<std::string> remarksOutput;
        bool isContinuation = false; 
        void clear();
    };
    
    // 【新增】用于存储时长规则的结构体
    struct DurationRule {
        int less_than_minutes;
        std::string value;
    };

    // 配置项
    std::string config_filepath_;
    std::string remark_prefix_;
    std::unordered_map<std::string, std::string> text_mapping_;
    std::unordered_map<std::string, std::string> text_duration_mapping_;
    std::vector<std::string> header_order_;
    // 【新增】存储时长映射规则的成员变量
    std::unordered_map<std::string, std::vector<DurationRule>> duration_mappings_;

    // 私有函数
    bool loadConfiguration();
    void writeDayData(std::ofstream& outFile, const DayData& day);
    std::string formatTime(const std::string& timeStrHHMM);
    bool isDateLine(const std::string& line);
    bool isRemarkLine(const std::string& line);
    bool parseEventLine(const std::string& line, std::string& outTimeStr, std::string& outDescription);
    // 【新增】计算分钟差的辅助函数
    int calculateDurationMinutes(const std::string& startTimeHHMM, const std::string& endTimeHHMM);

    // 核心逻辑拆分函数
    void processAndFormatEvents(DayData& day);
    void finalizeAndWriteDay(DayData& dayToFinalize, DayData& nextDay, std::ofstream& outFile);
    void handleDateLine(const std::string& line, DayData& currentDay, DayData& previousDay, std::ofstream& outFile, const std::string& year_prefix);
    void handleRemarkLine(const std::string& line, DayData& currentDay);
    void handleEventLine(const std::string& line, DayData& currentDay);
};

#endif // INTERVAL_PROCESSOR_H