// reprocessing/LogProcessor.cpp
#include "reprocessing/LogProcessor.hpp"

#include "reprocessing/validator/FileValidator.hpp"
#include "reprocessing/validator/common/ValidatorUtils.hpp"

#include "reprocessing/converter/facade/IntervalConverter.hpp"
#include "common/AnsiColors.hpp"

#include <iostream>
#include <algorithm>
#include <set>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

LogProcessor::LogProcessor(const AppConfig& config) : config_(config) {}


// --- [核心修改] ---
// 实现了新的 convertStreamToData 方法，它只负责转换
std::vector<InputData> LogProcessor::convertStreamToData(std::istream& combined_stream) {
    try {
        IntervalConverter processor(config_.interval_processor_config_path);
        return processor.executeConversion(combined_stream);
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "An error occurred during conversion: " << e.what() << RESET_COLOR << std::endl;
        return {}; // 返回空向量表示失败
    }
}


// --- [核心修改] ---
// processFile 现在只负责验证，不再有转换和输出验证的逻辑
ProcessingResult LogProcessor::processFile(const fs::path& source_file,
                                           const AppOptions& options)
{
    ProcessingResult result;
    result.success = true;
    
    if (options.validate_source) {
        // 验证逻辑保持不变
        FileValidator source_validator(config_.interval_processor_config_path);
        std::set<Error> errors;
        if (!source_validator.validate(source_file.string(), ValidatorType::Source, errors)) {
            printGroupedErrors(source_file.string(), errors);
            result.success = false;
        }
    }

    // 转换和输出验证的逻辑已移至 FilePipelineManager
    return result;
}


bool LogProcessor::collectFilesToProcess(const std::string& input_path_str, std::vector<fs::path>& out_files) {
    fs::path input_path(input_path_str);
    if (!fs::exists(input_path)) {
        std::cerr << RED_COLOR << "Error: Input path does not exist: " << input_path_str << RESET_COLOR << std::endl;
        return false;
    }

    if (fs::is_directory(input_path)) {
        for (const auto& entry : fs::recursive_directory_iterator(input_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                out_files.push_back(entry.path());
            }
        }
        std::sort(out_files.begin(), out_files.end());
    } else if (fs::is_regular_file(input_path)) {
        out_files.push_back(input_path);
    }
    return true;
}