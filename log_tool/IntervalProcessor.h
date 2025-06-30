// IntervalProcessor.h (已重构)

#ifndef INTERVAL_PROCESSOR_H
#define INTERVAL_PROCESSOR_H

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <fstream> //
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
    
    struct DurationRule {
        int less_than_minutes;
        std::string value;
    };

    // 配置项 (无变化)
    std::string config_filepath_;
    std::string remark_prefix_;
    std::unordered_map<std::string, std::string> text_mapping_;
    std::unordered_map<std::string, std::string> text_duration_mapping_;
    std::vector<std::string> header_order_;
    std::unordered_map<std::string, std::vector<DurationRule>> duration_mappings_;

    // 现有私有函数 (无变化)
    bool loadConfiguration();
    void writeDayData(std::ofstream& outFile, const DayData& day);
    std::string formatTime(const std::string& timeStrHHMM);
    bool isDateLine(const std::string& line);
    bool isRemarkLine(const std::string& line);
    bool parseEventLine(const std::string& line, std::string& outTimeStr, std::string& outDescription);
    int calculateDurationMinutes(const std::string& startTimeHHMM, const std::string& endTimeHHMM);

    // #### MODIFICATION: 新增的私有辅助函数，用于拆分 executeConversion 的职责 ####
    /**
     * @brief 将原始事件列表转换为格式化的备注行。
     * @param day 包含原始事件的 DayData 对象，其 remarksOutput 将被填充。
     */
    void processAndFormatEvents(DayData& day);

    /**
     * @brief 完成一天的处理并写入文件，包括计算与下一天的 sleep_night 关联。
     * @param dayToFinalize 需要被处理和写入的当天数据。
     * @param nextDay 第二天的数据，用于获取起床时间。
     * @param outFile 输出文件流。
     */
    void finalizeAndWriteDay(DayData& dayToFinalize, DayData& nextDay, std::ofstream& outFile);

    /**
     * @brief 处理源文件中的日期行，触发前一天的最终处理和写入。
     * @param line 包含日期的行。
     * @param currentDay 当前正在处理的数据块。
     * @param previousDay 前一天的数据块。
     * @param outFile 输出文件流。
     * @param year_prefix 年份前缀。
     */
    void handleDateLine(const std::string& line, DayData& currentDay, DayData& previousDay, std::ofstream& outFile, const std::string& year_prefix);

    /**
     * @brief 处理源文件中的备注行。
     * @param line 包含备注的行。
     * @param currentDay 当前正在处理的数据块。
     */
    void handleRemarkLine(const std::string& line, DayData& currentDay);

    /**
     * @brief 处理源文件中的事件行。
     * @param line 包含事件的行。
     * @param currentDay 当前正在处理的数据块。
     */
    void handleEventLine(const std::string& line, DayData& currentDay);
};

#endif // INTERVAL_PROCESSOR_H