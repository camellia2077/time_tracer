// converter/log_processor.cpp
#include "log_processor.hpp"

#include "converter/convert/facade/converter_service.hpp"
#include "common/ansi_colors.hpp"
#include <iostream>
#include <sstream>

LogProcessor::LogProcessor(const ConverterConfig& config) : config_(config) {}

void LogProcessor::convertStreamToData(std::istream& combined_stream, std::function<void(DailyLog&&)> data_consumer) {
    try {
        ConverterService processor(config_);
        processor.executeConversion(combined_stream, data_consumer);
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "An error occurred during conversion: " << e.what() << RESET_COLOR << std::endl;
    }
}

LogProcessingResult LogProcessor::processSourceContent(const std::string& /*filename*/, const std::string& content) {
    LogProcessingResult result;
    result.success = true;
    
    // [核心修改] 移除内部验证逻辑
    // 现在的 LogProcessor 假设输入内容已经被 Validator 模块（或 Pipeline 前置步骤）验证过了。
    // 它只专注于 "Content -> Struct" 的转换。
    
    try {
        std::stringstream ss(content);
        // 使用 lambda 捕获 result 并填充数据
        convertStreamToData(ss, [&](DailyLog&& log) {
             std::string key = log.date.substr(0, 7); // YYYY-MM
             result.processed_data[key].push_back(std::move(log));
        });
    } catch (...) {
        result.success = false;
    }

    return result;
}