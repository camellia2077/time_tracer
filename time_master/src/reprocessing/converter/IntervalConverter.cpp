// reprocessing/converter/IntervalConverter.cpp
#include "IntervalConverter.hpp"
#include "common/common_utils.hpp"
#include "reprocessing/converter/internal/Converter.hpp"
#include "reprocessing/converter/internal/InputParser.hpp"
#include "reprocessing/converter/internal/DayProcessor.hpp"
#include "reprocessing/converter/internal/OutputGenerator.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>

IntervalConverter::IntervalConverter(const std::string& config_filename) {
    if (!config_.load(config_filename)) {
        throw std::runtime_error("Failed to load IntervalConverter configuration.");
    }
}

bool IntervalConverter::executeConversion(const std::string& input_filepath, const std::string& output_filepath, const std::string& year_prefix) {
    std::ifstream inFile(input_filepath);
    if (!inFile.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open input file " << input_filepath << RESET_COLOR << std::endl;
        return false;
    }

    std::ofstream outFile(output_filepath);
    if (!outFile.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open output file " << output_filepath << RESET_COLOR << std::endl;
        return false;
    }

    InputParser parser(config_, year_prefix);
    Converter converter(config_);
    DayProcessor processor(converter);
    OutputGenerator generator;
    
    InputData previousDay;

    // InputParser 使用回调函数来处理每个解析出的 "天"
    parser.parse(inFile, [&](InputData& currentDay) {
        if (!previousDay.date.empty()) {
            // 当我们有了 "今天" 的数据，就可以最终处理并输出 "昨天" 的数据
            processor.process(previousDay, currentDay);
            generator.write(outFile, previousDay, config_);
        }
        previousDay = currentDay;
    });

    // 处理并输出文件中最后一个 "天" 的数据
    if (!previousDay.date.empty()) {
        InputData emptyNextDay; // 假设文件结束后没有新的一天
        processor.process(previousDay, emptyNextDay);
        generator.write(outFile, previousDay, config_);
    }

    return true;
}