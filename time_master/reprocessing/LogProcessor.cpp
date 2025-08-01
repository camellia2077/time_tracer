#include "common/pch.h"
// reprocessing/LogProcessor.cpp
#include "reprocessing/LogProcessor.h"

#include "reprocessing/validator/FileValidator.h"
#include "reprocessing/validator/ValidatorUtils.h"

#include "reprocessing/input_transfer/IntervalProcessor.h"
#include "common/common_utils.h"

#include <iostream>
#include <algorithm>
#include <set>
#include <ctime>
#include <filesystem>

#include <chrono>

namespace fs = std::filesystem;

LogProcessor::LogProcessor(const AppConfig& config) : config_(config) {}

// --- [MODIFIED] 函数返回类型和实现已更新 ---
ProcessingResult LogProcessor::processFile(const std::filesystem::path& source_file,
                                           const std::filesystem::path& output_file,
                                           const AppOptions& options)
{
    ProcessingResult result; // 初始化返回结果

    FileValidator validator(
        config_.interval_processor_config_path,
        config_.format_validator_config_path,
        config_.interval_processor_config_path
    );

    if (options.validate_source) {
        std::cout << "--- Validating source: " << source_file.string() << " ---\n";
        auto start_time = std::chrono::steady_clock::now();
        
        std::set<Error> errors;
        if (!validator.validate(source_file.string(), ValidatorType::Source, errors)) {
            printGroupedErrors(source_file.string(), errors, config_.error_log_path);
            result.success = false;
        } else {
             std::cout << GREEN_COLOR << "Source validation successful." << RESET_COLOR << std::endl;
        }
        
        auto end_time = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> duration = end_time - start_time;
        result.timings.validation_source_ms = duration.count(); // 存储时间
    }

    if (result.success && options.convert) {
        std::cout << "--- Converting: " << source_file.string() << " -> " << output_file.string() << " ---\n";
        auto start_time = std::chrono::steady_clock::now();

        IntervalProcessor processor(config_.interval_processor_config_path);
        std::string year_str = extractYearFromPath(source_file);
        if (!processor.executeConversion(source_file.string(), output_file.string(), year_str)) {
             result.success = false;
        } else {
            std::cout << GREEN_COLOR << "Conversion successful." << RESET_COLOR << std::endl;
        }

        auto end_time = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> duration = end_time - start_time;
        result.timings.conversion_ms = duration.count(); // 存储耗时
    }

    if (result.success && options.validate_output) {
        std::cout << "--- Validating output: " << output_file.string() << " ---\n";
        auto start_time = std::chrono::steady_clock::now();
        
        std::set<Error> errors;
        if (!validator.validate(output_file.string(), ValidatorType::Output, errors, options.enable_day_count_check)) {
            printGroupedErrors(output_file.string(), errors, config_.error_log_path);
            result.success = false;
        } else {
            std::cout << GREEN_COLOR << "Output validation successful." << RESET_COLOR << std::endl;
        }

        auto end_time = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> duration = end_time - start_time;
        result.timings.validation_output_ms = duration.count(); // 存储耗时
    }

    return result; // 返回包含成功标志和所有计时数据的结构体
}

// Corrected function signature to match the header file
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

std::string LogProcessor::extractYearFromPath(const fs::path& file_path) {
    fs::path current_path = file_path.parent_path();
    auto is_four_digit_string = [](const std::string& s) {
        return s.length() == 4 && std::all_of(s.begin(), s.end(), ::isdigit);
    };
    while (!current_path.empty() && current_path.has_filename()) {
        if (is_four_digit_string(current_path.filename().string())) {
            return current_path.filename().string();
        }
        current_path = current_path.parent_path();
    }
    std::time_t now = std::time(nullptr);
    std::tm* ltm = std::localtime(&now);
    return std::to_string(1900 + ltm->tm_year);
}