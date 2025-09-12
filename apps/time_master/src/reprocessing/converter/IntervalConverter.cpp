// reprocessing/converter/IntervalConverter.cpp
#include "IntervalConverter.hpp"
#include "common/AnsiColors.hpp"

#include "reprocessing/converter/pipelines/converter/Converter.hpp"
#include "reprocessing/converter/pipelines/InputParser.hpp"
#include "reprocessing/converter/pipelines/DayProcessor.hpp"

#include <iostream>
#include <stdexcept>

IntervalConverter::IntervalConverter(const std::string& config_filename) {
    if (!config_.load(config_filename)) {
        throw std::runtime_error("Failed to load IntervalConverter configuration.");
    }
}

// --- [核心修改] ---
// 实现了新的 executeConversion，它现在返回处理好的数据，而不是写入文件
std::vector<InputData> IntervalConverter::executeConversion(std::istream& combined_input_stream) {
    InputParser parser(config_);
    std::vector<InputData> all_days;

    parser.parse(combined_input_stream, [&](InputData& day) {
        all_days.push_back(day);
    });

    if (all_days.empty()) {
        return {}; // 如果没有数据，返回一个空向量
    }

    Converter converter(config_);
    DayProcessor processor(converter);
    InputData empty_prev_day; 
    
    for (size_t i = 0; i < all_days.size(); ++i) {
        InputData& previous_day = (i > 0) ? all_days[i - 1] : empty_prev_day;
        InputData& current_day = all_days[i];
        processor.process(previous_day, current_day);
    }
    
    // --- [核心修改] 移除用于生成睡眠的占位天 ---
    // 检查第一个元素是否为占位天（例如，y2024年的1231日，用于生成y2025年0101日的睡眠记录）
    // 这种情况的特征是：
    // 1. 至少有两天的数据。
    // 2. 第一天的月份是12月，第二天的月份是1月。
    // 3. 第二天的年份比第一天大1。
    if (all_days.size() > 1) {
        const auto& first_day = all_days[0];
        const auto& second_day = all_days[1];

        if (first_day.date.length() == 8 && second_day.date.length() == 8) {
            std::string first_month = first_day.date.substr(4, 2);
            std::string second_month = second_day.date.substr(4, 2);

            if (first_month == "12" && second_month == "01") {
                int first_year = std::stoi(first_day.date.substr(0, 4));
                int second_year = std::stoi(second_day.date.substr(0, 4));

                if (second_year == first_year + 1) {
                    // 确认是占位天，将其从结果中移除
                    all_days.erase(all_days.begin());
                }
            }
        }
    }
    
    // 返回包含所有已处理天数的向量
    return all_days;
}