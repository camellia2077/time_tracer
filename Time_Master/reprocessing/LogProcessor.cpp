#include "LogProcessor.h"
#include "FormatValidator.h"
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

    if (options_.run_all) {
        return runAllInOneMode();
    } else {
        return runIndividualMode();
    }
}

bool LogProcessor::runAllInOneMode() {
    // ... 此函数保持不变 ...
    std::cout << "\n--- 运行 \"all-in-one\" 模式 (错误即停) ---\n" << std::endl;
    std::vector<fs::path> files_to_process;
    if (!collectFilesToProcess(files_to_process)) return false;
    FormatValidator validator(config_.format_validator_config_path, config_.interval_processor_config_path, options_.enable_day_count_check);
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
        std::set<FormatValidator::Error> errors;
        if (!validator.validateSourceFile(file.string(), errors)) {
            std::cerr << RED_COLOR << "错误: 源文件 " << file.string() << " 检验失败。" << RESET_COLOR << std::endl;
            FormatValidator::printGroupedErrors(file.string(), errors, config_.error_log_path);
            std::cerr << "\n程序已终止。" << std::endl;
            return false;
        }
    }
    std::cout << GREEN_COLOR << "所有源文件检验通过。" << RESET_COLOR << std::endl;
    std::cout << "\n--- 阶段 2: 转换所有文件... ---" << std::endl;
    for (const auto& file : files_to_process) {
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
            std::cerr << "\n程序已终止。" << std::endl;
            return false;
        }
        source_to_output_map[file] = target_path;
    }
    std::cout << GREEN_COLOR << "所有文件转换成功。" << RESET_COLOR << std::endl;
    std::cout << "\n--- 阶段 3: 检验所有输出文件... ---" << std::endl;
    for (const auto& pair : source_to_output_map) {
        const fs::path& output_file = pair.second;
        std::set<FormatValidator::Error> errors;
        if (!validator.validateOutputFile(output_file.string(), errors)) {
            std::cerr << RED_COLOR << "错误: 输出文件 " << output_file.string() << " 检验失败。" << RESET_COLOR << std::endl;
            FormatValidator::printGroupedErrors(output_file.string(), errors, config_.error_log_path);
            std::cerr << "\n程序已终止。" << std::endl;
            return false;
        }
    }
    std::cout << GREEN_COLOR << "所有输出文件检验通过。" << RESET_COLOR << std::endl;
    std::cout << "\n--- 所有流程成功完成！ ---\n" << std::endl;
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

    // 【核心修改】增加一个特殊模式处理
    // 如果只有 validate_output 为 true，那么直接验证输入路径下的文件
    bool validate_output_only = options_.validate_output && !options_.validate_source && !options_.convert;

    for (const auto& file : files_to_process) {
        std::cout << "\n=======================================================\n";
        
        // 如果是仅验证输出模式，直接将 file 视为目标文件
        if (validate_output_only) {
            std::cout << "正在检验输出文件: " << file.string() << "\n";
            FormatValidator validator(config_.format_validator_config_path, config_.interval_processor_config_path, options_.enable_day_count_check);
            std::set<FormatValidator::Error> errors;
            if (validator.validateOutputFile(file.string(), errors)) {
                v_output_success_++;
                std::cout << GREEN_COLOR << "成功: 输出文件格式合规。" << RESET_COLOR << std::endl;
            } else {
                v_output_fail_++;
                std::cerr << RED_COLOR << "失败: 输出文件 " << file.string() << " 格式错误。" << RESET_COLOR << std::endl;
                FormatValidator::printGroupedErrors(file.string(), errors, config_.error_log_path);
            }
            continue; // 处理完一个文件后直接进入下一次循环
        }

        // --- 以下是原来的逻辑 ---
        std::cout << "正在处理源文件: " << file.string() << "\n";
        
        bool current_file_ok = true;
        fs::path final_output_path;

        if (options_.validate_source) {
            std::cout << "--- 阶段 1: 检验源文件 ---" << std::endl;
            FormatValidator validator(config_.format_validator_config_path, config_.interval_processor_config_path, options_.enable_day_count_check);
            std::set<FormatValidator::Error> errors;
            if (validator.validateSourceFile(file.string(), errors)) {
                v_source_success_++;
                std::cout << GREEN_COLOR << "成功: 源文件格式合规。" << RESET_COLOR << std::endl;
            } else {
                v_source_fail_++;
                current_file_ok = false;
                std::cerr << RED_COLOR << "失败: 源文件格式错误。" << RESET_COLOR << std::endl;
                FormatValidator::printGroupedErrors(file.string(), errors, config_.error_log_path);
            }
        }

        if (options_.convert && current_file_ok) {
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
                std::cerr << RED_COLOR << "失败: 文件转换过程中发生错误。" << RESET_COLOR << std::endl;
            }
        }

        if (options_.validate_output && current_file_ok) {
            // 如果 convert 没有执行，final_output_path 是空的，这里会出错
            // 所以，常规流程下，validate_output 必须和 convert 一起使用
            if (options_.convert) {
                 std::cout << "--- 阶段 3: 检验输出文件 ---" << std::endl;
                FormatValidator validator(config_.format_validator_config_path, config_.interval_processor_config_path, options_.enable_day_count_check);
                std::set<FormatValidator::Error> errors;
                if (validator.validateOutputFile(final_output_path.string(), errors)) {
                    v_output_success_++;
                    std::cout << GREEN_COLOR << "成功: 输出文件格式合规。" << RESET_COLOR << std::endl;
                } else {
                    v_output_fail_++;
                    std::cerr << RED_COLOR << "失败: 输出文件 " << final_output_path.string() << " 格式错误。" << RESET_COLOR << std::endl;
                    FormatValidator::printGroupedErrors(final_output_path.string(), errors, config_.error_log_path);
                }
            }
        }
    }

    printSummary();
    return (v_source_fail_ + convert_fail_ + v_output_fail_) == 0;
}


bool LogProcessor::collectFilesToProcess(std::vector<fs::path>& out_files) {
    // ... 此函数保持不变 ...
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
        if (out_files.empty()) {
            std::cout << YELLOW_COLOR << "警告: 在文件夹 " << options_.input_path << " 中未找到 .txt 文件。" << RESET_COLOR << std::endl;
        }
        std::sort(out_files.begin(), out_files.end());
    } else {
        out_files.push_back(input_path);
    }
    return true;
}

std::string LogProcessor::extractYearFromPath(const fs::path& file_path) {
    // ... 此函数保持不变 ...
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
    // ... 此函数保持不变 ...
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