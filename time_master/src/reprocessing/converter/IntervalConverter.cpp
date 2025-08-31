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
#include <vector>

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

    // 步骤 1: [收集] 解析整个文件，将所有天的数据存入向量
    // =================================================================
    InputParser parser(config_, year_prefix);
    std::vector<InputData> all_days;
    parser.parse(inFile, [&](InputData& day) {
        all_days.push_back(day);
    });

    if (all_days.empty()) {
        // 如果文件为空或无效，创建一个空的输出文件然后成功退出
        std::ofstream outFile(output_filepath);
        outFile << "[]" << std::endl;
        return true;
    }

    // 步骤 2: [处理] 遍历向量，处理天与天之间的关联
    // =================================================================
    Converter converter(config_);
    DayProcessor processor(converter);
    
    // [修复] 在循环外创建一个空的 InputData 对象
    InputData empty_next_day; 

    for (size_t i = 0; i < all_days.size(); ++i) {
        InputData& current_day = all_days[i];
        
        // [修复] 使用预先创建的 empty_next_day 来避免绑定临时对象
        InputData& next_day = (i + 1 < all_days.size()) ? all_days[i + 1] : empty_next_day;
        
        processor.process(current_day, next_day);
    }
    
    // 步骤 3: [输出] 将处理完毕的整个向量一次性写入JSON文件
    // =================================================================
    std::ofstream outFile(output_filepath);
    if (!outFile.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open output file " << output_filepath << RESET_COLOR << std::endl;
        return false;
    }
    OutputGenerator generator;
    generator.write(outFile, all_days, config_);

    return true;
}