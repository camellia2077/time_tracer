// reprocessing/converter/IntervalConverter.cpp
#include "IntervalConverter.hpp"
#include "common/AnsiColors.hpp" // For colored console output

#include "reprocessing/converter/internal/Converter.hpp"
#include "reprocessing/converter/internal/InputParser.hpp"
#include "reprocessing/converter/internal/DayProcessor.hpp"
#include "reprocessing/converter/internal/OutputGenerator.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

IntervalConverter::IntervalConverter(const std::string& config_filename) {
    if (!config_.load(config_filename)) {
        throw std::runtime_error("Failed to load IntervalConverter configuration.");
    }
}

// [修改] 移除了 year_prefix 参数
bool IntervalConverter::executeConversion(const std::string& input_filepath, const std::string& output_filepath) {
    std::ifstream inFile(input_filepath);
    if (!inFile.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open input file " << input_filepath << RESET_COLOR << std::endl;
        return false;
    }

    // [修改] InputParser 的构造不再需要年份
    InputParser parser(config_);
    std::vector<InputData> all_days;
    parser.parse(inFile, [&](InputData& day) {
        all_days.push_back(day);
    });

    if (all_days.empty()) {
        std::ofstream outFile(output_filepath);
        outFile << "[]" << std::endl;
        return true;
    }

    Converter converter(config_);
    DayProcessor processor(converter);
    InputData empty_next_day; 
    for (size_t i = 0; i < all_days.size(); ++i) {
        InputData& current_day = all_days[i];
        InputData& next_day = (i + 1 < all_days.size()) ? all_days[i + 1] : empty_next_day;
        processor.process(current_day, next_day);
    }
    
    std::ofstream outFile(output_filepath);
    if (!outFile.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open output file " << output_filepath << RESET_COLOR << std::endl;
        return false;
    }
    OutputGenerator generator;
    generator.write(outFile, all_days, config_);

    return true;
}