// reprocessing/LogProcessor.cpp
#include "reprocessing/LogProcessor.hpp"

#include "reprocessing/validator/FileValidator.hpp"
#include "reprocessing/validator/common/ValidatorUtils.hpp"

#include "reprocessing/converter/IntervalConverter.hpp"
#include "common/AnsiColors.hpp" // For colored console output

#include <iostream>
#include <algorithm>
#include <set>
#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;

LogProcessor::LogProcessor(const AppConfig& config) : config_(config) {}

ProcessingResult LogProcessor::processFile(const std::filesystem::path& source_file,
                                           const std::filesystem::path& output_file,
                                           const AppOptions& options)
{
    ProcessingResult result;
    result.success = true;

    if (options.validate_source) {
        std::cout << "--- Validating source: " << source_file.string() << " ---\n";
        auto start_time = std::chrono::steady_clock::now();
        
        FileValidator source_validator(config_.interval_processor_config_path);
        std::set<Error> errors;
        
        if (!source_validator.validate(source_file.string(), ValidatorType::Source, errors)) {
            printGroupedErrors(source_file.string(), errors, config_.error_log_path);
            result.success = false;
        } else {
             std::cout << GREEN_COLOR << "Source validation successful." << RESET_COLOR << std::endl;
        }
        
        auto end_time = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> duration = end_time - start_time;
        result.timings.validation_source_ms = duration.count();
    }

    if (result.success && options.convert) {
        std::cout << "--- Converting: " << source_file.string() << " -> " << output_file.string() << " ---\n";
        auto start_time = std::chrono::steady_clock::now();

        try {
            // [修改] 调用新的构造函数，传入 config_ (即 AppConfig)
            IntervalConverter processor(config_.interval_processor_config_path, config_);
            // [核心修改] 不再从路径提取年份，直接调用转换
            if (!processor.executeConversion(source_file.string(), output_file.string())) {
                 result.success = false;
            } else {
                std::cout << GREEN_COLOR << "Conversion successful." << RESET_COLOR << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << RED_COLOR << "An error occurred during conversion: " << e.what() << RESET_COLOR << std::endl;
            result.success = false;
        }

        auto end_time = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> duration = end_time - start_time;
        result.timings.conversion_ms = duration.count();
    }

    if (result.success && options.validate_output) {
        std::cout << "--- Validating output: " << output_file.string() << " ---\n";
        auto start_time = std::chrono::steady_clock::now();
        
        FileValidator output_validator(config_.interval_processor_config_path);
        std::set<Error> errors;
        
        if (!output_validator.validate(output_file.string(), ValidatorType::JsonOutput, errors, options.enable_day_count_check)) {
            printGroupedErrors(output_file.string(), errors, config_.error_log_path);
            result.success = false;
        } else {
            std::cout << GREEN_COLOR << "Output validation successful." << RESET_COLOR << std::endl;
        }

        auto end_time = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> duration = end_time - start_time;
        result.timings.validation_output_ms = duration.count();
    }

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