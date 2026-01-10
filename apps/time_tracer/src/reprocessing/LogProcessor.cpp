// reprocessing/LogProcessor.cpp
#include "reprocessing/LogProcessor.hpp"

#include "reprocessing/validator/FileValidator.hpp"
#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include "reprocessing/converter/facade/IntervalConverter.hpp"
#include "common/AnsiColors.hpp"

#include <iostream>
#include <set>
#include <filesystem>
#include <fstream> // 仅 processFile 可能会用到，或者被底层 validator 使用

namespace fs = std::filesystem;

LogProcessor::LogProcessor(const AppConfig& config) : config_(config) {}

std::vector<InputData> LogProcessor::convertStreamToData(std::istream& combined_stream) {
    try {
        IntervalConverter processor(config_.interval_processor_config_path);
        return processor.executeConversion(combined_stream);
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "An error occurred during conversion: " << e.what() << RESET_COLOR << std::endl;
        return {}; 
    }
}

ProcessingResult LogProcessor::processFile(const fs::path& source_file,
                                           const AppOptions& options)
{
    ProcessingResult result;
    result.success = true;
    
    if (options.validate_source) {
        FileValidator source_validator(config_.interval_processor_config_path);
        std::set<Error> errors;
        if (!source_validator.validate(source_file, ValidatorType::Source, errors)) {
            printGroupedErrors(source_file.string(), errors); 
            // 验证失败不一定中断流程，视业务需求而定，这里保持原样
            result.success = false; 
        }
    }
    return result;
}

