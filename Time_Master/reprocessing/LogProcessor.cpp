#include "LogProcessor.h"
// 【修改】：包含新的、分离的验证器头文件
#include "SourceFileValidator.h"
#include "OutputFileValidator.h"
#include "ValidatorUtils.h" // 需要它来访问 printGroupedErrors 和 Error 结构

#include "IntervalProcessor.h"
#include "common_utils.h"
#include <iostream>
#include <algorithm>
#include <map>
#include <set>
#include <ctime>
#include <filesystem>

namespace fs = std::filesystem;

LogProcessor::LogProcessor(const AppConfig& config) : config_(config) {}

bool LogProcessor::run(const AppOptions& options) {
    options_ = options;
    v_source_success_ = 0;
    v_source_fail_ = 0;
    convert_success_ = 0;
    convert_fail_ = 0;
    v_output_success_ = 0;
    v_output_fail_ = 0;

    return options_.run_all ? runAllInOneMode() : runIndividualMode();
}

bool LogProcessor::runAllInOneMode() {
    std::cout << "\n--- 运行 \"all-in-one\" 模式 (错误即停) ---\n" << std::endl;
    std::vector<fs::path> files_to_process;
    if (!collectFilesToProcess(files_to_process)) return false;

    // 【修改】：分别创建源文件和输出文件验证器
    SourceFileValidator source_validator(config_.interval_processor_config_path); // 源文件验证器需要 remark_prefix 配置
    OutputFileValidator output_validator(config_.format_validator_config_path, config_.interval_processor_config_path, options_.enable_day_count_check);

    IntervalProcessor processor(config_.interval_processor_config_path);
    std::map<fs::path, fs::path> source_to_output_map;
    fs::path input_root(options_.input_path);
    fs::path output_root_path;
    bool is_dir = fs::is_directory(input_root);
     if (is_dir) {
        output_root_path = input_root.parent_path() / ("Processed_" + input_root.filename().string());
        fs::create_directories(output_root_path);
    }

    std::cout << "--- 阶段 1: 检验所有源文件... ---" << std::endl;
    for (const auto& file : files_to_process) {
        std::set<Error> errors; // 【修改】: 使用共享的 Error 类型
        if (!source_validator.validate(file.string(), errors)) { // 【修改】: 调用 source_validator
            std::cerr << RED_COLOR << "错误: 源文件 " << file.string() << " 检验失败。" << RESET_COLOR << std::endl;
            printGroupedErrors(file.string(), errors, config_.error_log_path); // 【修改】: 调用共享的打印函数
            std::cerr << "\n程序已终止。" << std::endl;
            return false;
        }
    }

    std::cout << GREEN_COLOR << "所有源文件检验通过。" << RESET_COLOR << std::endl;
    std::cout << "\n--- 阶段 2: 转换所有文件... ---" << std::endl;
    for (const auto& file : files_to_process) {
        // ... 转换逻辑不变 ...
        fs::path target_path;
        if (is_dir) {
            target_path = output_root_path / fs::relative(file, input_root);
            fs::create_directories(target_path.parent_path());
        } else {
            target_path = "Processed_" + file.filename().string();
        }
        std::string year_str = extractYearFromPath(file);
        if (!processor.executeConversion(file.string(), target_path.string(), year_str)) {
            std::cerr << RED_COLOR << "错误: 转换文件 " << file.string() << " 失败。" << RESET_COLOR << std::endl;
            return false;
        }
        source_to_output_map[file] = target_path;
    }

    std::cout << GREEN_COLOR << "所有文件转换成功。" << RESET_COLOR << std::endl;
    std::cout << "\n--- 阶段 3: 检验所有输出文件... ---" << std::endl;
    for (const auto& pair : source_to_output_map) {
        const fs::path& output_file = pair.second;
        std::set<Error> errors; // 【修改】: 使用共享的 Error 类型
        if (!output_validator.validate(output_file.string(), errors)) { // 【修改】: 调用 output_validator
            std::cerr << RED_COLOR << "错误: 输出文件 " << output_file.string() << " 检验失败。" << RESET_COLOR << std::endl;
            printGroupedErrors(output_file.string(), errors, config_.error_log_path); // 【修改】: 调用共享的打印函数
            std::cerr << "\n程序已终止。" << std::endl;
            return false;
        }
    }
    std::cout << GREEN_COLOR << "所有输出文件检验通过。" << RESET_COLOR << std::endl;
    return true;
}

bool LogProcessor::runIndividualMode() {
    std::vector<fs::path> files_to_process;
    if (!collectFilesToProcess(files_to_process)) return false;

    fs::path input_root(options_.input_path);
    fs::path output_root_path;
    bool is_dir = fs::is_directory(input_root);
    if (is_dir && options_.convert) {
        output_root_path = input_root.parent_path() / ("Processed_" + input_root.filename().string());
        fs::create_directories(output_root_path);
    }

    bool validate_output_only = options_.validate_output && !options_.validate_source && !options_.convert;

    for (const auto& file : files_to_process) {
        std::cout << "\n=======================================================\n";
        
        if (validate_output_only) {
            std::cout << "正在检验输出文件: " << file.string() << "\n";
            // 【修改】: 创建并使用 OutputFileValidator
            OutputFileValidator validator(config_.format_validator_config_path, config_.interval_processor_config_path, options_.enable_day_count_check);
            std::set<Error> errors;
            if (validator.validate(file.string(), errors)) {
                v_output_success_++;
                std::cout << GREEN_COLOR << "成功: 输出文件格式合规。" << RESET_COLOR << std::endl;
            } else {
                v_output_fail_++;
                std::cerr << RED_COLOR << "失败: 输出文件 " << file.string() << " 格式错误。" << RESET_COLOR << std::endl;
                printGroupedErrors(file.string(), errors, config_.error_log_path);
            }
            continue;
        }

        std::cout << "正在处理源文件: " << file.string() << "\n";
        bool current_file_ok = true;
        fs::path final_output_path;

        if (options_.validate_source) {
            std::cout << "--- 阶段 1: 检验源文件 ---" << std::endl;
            // 【修改】: 创建并使用 SourceFileValidator
            SourceFileValidator validator(config_.interval_processor_config_path);
            std::set<Error> errors;
            if (validator.validate(file.string(), errors)) {
                v_source_success_++;
                std::cout << GREEN_COLOR << "成功: 源文件格式合规。" << RESET_COLOR << std::endl;
            } else {
                v_source_fail_++;
                current_file_ok = false;
                std::cerr << RED_COLOR << "失败: 源文件格式错误。" << RESET_COLOR << std::endl;
                printGroupedErrors(file.string(), errors, config_.error_log_path);
            }
        }

        if (options_.convert && current_file_ok) {
            // ... 转换逻辑不变 ...
            std::cout << "--- 阶段 2: 转换文件 ---" << std::endl;
            if (is_dir) {
                final_output_path = output_root_path / fs::relative(file, input_root);
                fs::create_directories(final_output_path.parent_path());
            } else {
                final_output_path = "Processed_" + file.filename().string();
            }
            IntervalProcessor processor(config_.interval_processor_config_path);
            std::string year_str = extractYearFromPath(file);
            if (processor.executeConversion(file.string(), final_output_path.string(), year_str)) {
                convert_success_++;
                std::cout << GREEN_COLOR << "成功: 文件已转换为 " << final_output_path.string() << RESET_COLOR << std::endl;
            } else {
                convert_fail_++;
                current_file_ok = false;
            }
        }

        if (options_.validate_output && current_file_ok && options_.convert) {
            std::cout << "--- 阶段 3: 检验输出文件 ---" << std::endl;
            // 【修改】: 创建并使用 OutputFileValidator
            OutputFileValidator validator(config_.format_validator_config_path, config_.interval_processor_config_path, options_.enable_day_count_check);
            std::set<Error> errors;
            if (validator.validate(final_output_path.string(), errors)) {
                v_output_success_++;
                std::cout << GREEN_COLOR << "成功: 输出文件格式合规。" << RESET_COLOR << std::endl;
            } else {
                v_output_fail_++;
                std::cerr << RED_COLOR << "失败: 输出文件 " << final_output_path.string() << " 格式错误。" << RESET_COLOR << std::endl;
                printGroupedErrors(final_output_path.string(), errors, config_.error_log_path);
            }
        }
    }

    printSummary();
    return (v_source_fail_ + convert_fail_ + v_output_fail_) == 0;
}

// --- 以下辅助函数保持不变 ---
bool LogProcessor::collectFilesToProcess(std::vector<fs::path>& out_files) {
    fs::path input_path(options_.input_path);
    if (!fs::exists(input_path)) {
        std::cerr << RED_COLOR << "错误: 输入的路径不存在: " << options_.input_path << RESET_COLOR << std::endl;
        return false;
    }
    if (fs::is_directory(input_path)) {
        for (const auto& entry : fs::recursive_directory_iterator(input_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                out_files.push_back(entry.path());
            }
        }
        std::sort(out_files.begin(), out_files.end());
    } else {
        out_files.push_back(input_path);
    }
    return true;
}

std::string LogProcessor::extractYearFromPath(const fs::path& file_path) {
    // ... 实现不变 ...
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

void LogProcessor::printSummary() const {
    // ... 实现不变 ...
    std::cout << "\n\n--- 所有任务处理完毕 ---\n";
    if (options_.validate_source) {
        std::cout << "  - 源文件检验: " << GREEN_COLOR << v_source_success_ << " 成功" << RESET_COLOR << ", " << RED_COLOR << v_source_fail_ << " 失败" << RESET_COLOR << std::endl;
    }
    if (options_.convert) {
        std::cout << "  - 文件转换:   " << GREEN_COLOR << convert_success_ << " 成功" << RESET_COLOR << ", " << RED_COLOR << convert_fail_ << " 失败" << RESET_COLOR << std::endl;
    }
    if (options_.validate_output) {
        std::cout << "  - 输出文件检验: " << GREEN_COLOR << v_output_success_ << " 成功" << RESET_COLOR << ", " << RED_COLOR << v_output_fail_ << " 失败" << RESET_COLOR << std::endl;
    }
}