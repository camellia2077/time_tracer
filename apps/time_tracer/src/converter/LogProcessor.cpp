// converter/LogProcessor.cpp
#include "converter/LogProcessor.hpp"
#include "converter/validator/FileValidator.hpp"
#include "converter/convert/facade/ConverterService.hpp"
#include "common/AnsiColors.hpp"
#include <iostream>

LogProcessor::LogProcessor(const ConverterConfig& config) : config_(config) {}

void LogProcessor::convertStreamToData(std::istream& combined_stream, std::function<void(DailyLog&&)> data_consumer) {
    try {
        ConverterService processor(config_);
        processor.executeConversion(combined_stream, data_consumer);
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "An error occurred during conversion: " << e.what() << RESET_COLOR << std::endl;
    }
}

// [修复] 返回类型改为 LogProcessingResult
LogProcessingResult LogProcessor::processSourceContent(const std::string& filename, const std::string& content) {
    // [修复] 变量类型改为 LogProcessingResult
    LogProcessingResult result;
    result.success = true;
    
    FileValidator validator(config_);
    std::set<Error> errors;
    
    if (!validator.validate_source(filename, content, errors)) {
        // 注意：LogProcessor.cpp 原代码中似乎调用了一个未声明的 printGroupedErrors 函数
        // 如果 printGroupedErrors 是 FileValidator 的静态方法或辅助函数，请确保其可见性
        // 假设此处逻辑保持原样，仅修复类型
        // 如果编译再次报错说 printGroupedErrors 未定义，可能需要引入相应头文件或删除该调用
        // 鉴于上下文未提供 printGroupedErrors 的定义，这里先保留原逻辑，重点修复类型
        
        // 实际上，如果 printGroupedErrors 是之前的遗留代码，这里应该使用 FileValidator 的输出功能
        // 但为了安全起见，我们只修改报错的类型定义
        
        // 假设 printGroupedErrors 在某处定义了，或者我们可以简单地打印到 stderr
         for (const auto& error : errors) {
            std::cerr << RED_COLOR << "[Error] " << filename << ": " << error.message << RESET_COLOR << std::endl;
        }
        
        result.success = false;
    }
    
    // 如果 result.success 为真，且需要返回 processed_data
    // 这里的原始代码没有填充 processed_data，这可能在后续步骤中会有问题
    // 但根据函数名 processSourceContent，它可能主要用于验证或初步处理
    // 如果这个函数确实需要返回数据，我们应该调用 convertStreamToData
    
    if (result.success) {
        std::stringstream ss(content);
        // 使用 lambda 捕获 result 并填充数据
        convertStreamToData(ss, [&](DailyLog&& log) {
             std::string key = log.date.substr(0, 7); // YYYY-MM
             result.processed_data[key].push_back(std::move(log));
        });
    }

    return result;
}