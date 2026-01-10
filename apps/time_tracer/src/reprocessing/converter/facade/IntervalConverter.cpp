// reprocessing/converter/facade/IntervalConverter.cpp
#include "IntervalConverter.hpp"
#include "common/AnsiColors.hpp"

#include "reprocessing/converter/pipelines/InputParser.hpp"
#include "reprocessing/converter/pipelines/DayProcessor.hpp"

#include <iostream>
#include <stdexcept> 

IntervalConverter::IntervalConverter(const std::filesystem::path& config_path) {
    if (!config_.load(config_path)) { 
        throw std::runtime_error("Failed to load IntervalConverter configuration.");
    }
}

std::vector<InputData> IntervalConverter::executeConversion(std::istream& combined_input_stream) {
    InputParser parser(config_);
    std::vector<InputData> all_days;

    parser.parse(combined_input_stream, [&](InputData& day) {
        all_days.push_back(day);
    });

    if (all_days.empty()) {
        return {};
    }

    DayProcessor processor(config_);
    InputData empty_prev_day;

    for (size_t i = 0; i < all_days.size(); ++i) {
        InputData& previous_day = (i > 0) ? all_days[i - 1] : empty_prev_day;
        InputData& current_day = all_days[i];
        processor.process(previous_day, current_day);
    }
    
    // 移除用于生成睡眠的占位天
    if (all_days.size() > 1) {
        const auto& first_day = all_days[0];
        const auto& second_day = all_days[1];

        // [核心修改] 适配 YYYY-MM-DD 长度 (10) 和索引
        if (first_day.date.length() == 10 && second_day.date.length() == 10) {
            // YYYY-MM-DD
            // 0123456789
            // 提取月份: 索引 5, 长度 2
            std::string first_month = first_day.date.substr(5, 2);
            std::string second_month = second_day.date.substr(5, 2);

            if (first_month == "12" && second_month == "01") {
                int first_year = std::stoi(first_day.date.substr(0, 4));
                int second_year = std::stoi(second_day.date.substr(0, 4));

                if (second_year == first_year + 1) {
                    all_days.erase(all_days.begin());
                }
            }
        }
    }
    
    return all_days;
}