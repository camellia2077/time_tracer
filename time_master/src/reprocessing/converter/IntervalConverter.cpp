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
    
    // 返回包含所有已处理天数的向量
    return all_days;
}