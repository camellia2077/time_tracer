// file: action_handler/file/FilePipelineManager.cpp

#include "common/pch.h"
#include "FilePipelineManager.h"
#include "common/common_utils.h"
#include "file_handler/FileUtils.h"
#include <iostream>
#include <iomanip>

namespace fs = std::filesystem;

/**
 * @brief 构造函数实现
 */
FilePipelineManager::FilePipelineManager(const AppConfig& config)
    : processor_(config), app_config_(config) {}

/**
 * @brief 运行完整的处理流水线
 */
std::optional<fs::path> FilePipelineManager::run(const std::string& input_path) {
    if (!collectFiles(input_path) || !validateSourceFiles() || !convertFiles() || !validateOutputFiles(true)) {
        return std::nullopt; // 如果任何一个阶段失败，则返回空
    }

    // 如果成功，计算并返回输出根目录
    if (fs::is_directory(input_root_)) {
        return input_root_.parent_path() / ("Processed_" + input_root_.filename().string());
    }
    // 如果是单个文件，技术上没有 "根" 目录，但可以返回其父目录
    return input_root_.parent_path();
}

/**
 * @brief 阶段一：收集需要处理的文件
 */
bool FilePipelineManager::collectFiles(const std::string& input_path) {
    input_root_ = fs::path(input_path);
    if (!fs::exists(input_root_)) {
        std::cerr << RED_COLOR << "错误: 输入的路径不存在: " << input_path << RESET_COLOR << std::endl;
        return false;
    }

    files_to_process_.clear();
    source_to_output_map_.clear();

    files_to_process_ = FileUtils::find_files_by_extension_recursively(input_root_, ".txt");

    if (fs::is_regular_file(input_root_) && input_root_.extension() == ".txt") {
        if (files_to_process_.empty()) {
            files_to_process_.push_back(input_root_);
        }
    }

    std::cout << "信息: 成功收集到 " << files_to_process_.size() << " 个待处理文件。" << std::endl;
    return !files_to_process_.empty();
}

/**
 * @brief 阶段二：检验源文件
 */
bool FilePipelineManager::validateSourceFiles() {
    const std::string current_operation_name = "validateSourceFiles";
    std::cout << "\n--- 阶段: 检验源文件 ---" << std::endl;
    if (files_to_process_.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 没有已收集的文件可供检验。" << RESET_COLOR << std::endl;
        return true;
    }

    bool all_ok = true;
    double total_validation_time_ms = 0.0;

    for (const auto& file : files_to_process_) {
        AppOptions opts;
        opts.validate_source = true;

        ProcessingResult result = processor_.processFile(file, "", opts);
        total_validation_time_ms += result.timings.validation_source_ms;

        if (!result.success) {
            all_ok = false;
        }
    }

    printTimingStatistics(current_operation_name, total_validation_time_ms);
    std::cout << (all_ok ? GREEN_COLOR : RED_COLOR) << "源文件检验阶段 " << (all_ok ? "全部通过" : "存在失败项") << "。" << RESET_COLOR << std::endl;
    return all_ok;
}

/**
 * @brief 阶段三：转换文件
 */
bool FilePipelineManager::convertFiles() {
    const std::string current_operation_name = "convertFiles";
    std::cout << "\n--- 阶段: 转换文件 ---" << std::endl;
    if (files_to_process_.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 没有已收集的文件可供转换。" << RESET_COLOR << std::endl;
        return true;
    }

    bool is_dir = fs::is_directory(input_root_);
    fs::path output_root_path;

    if (is_dir) {
        output_root_path = input_root_.parent_path() / ("Processed_" + input_root_.filename().string());
        fs::create_directories(output_root_path);
    }

    bool all_ok = true;
    double total_conversion_time_ms = 0.0;

    for (const auto& file : files_to_process_) {
        fs::path output_file_path;
        if (is_dir) {
            output_file_path = output_root_path / fs::relative(file, input_root_);
            fs::create_directories(output_file_path.parent_path());
        } else {
            output_file_path = input_root_.parent_path() / ("Processed_" + file.filename().string());
        }

        AppOptions opts;
        opts.convert = true;

        ProcessingResult result = processor_.processFile(file, output_file_path, opts);
        total_conversion_time_ms += result.timings.conversion_ms;

        if (result.success) {
            source_to_output_map_[file] = output_file_path;
        } else {
            all_ok = false;
        }
    }

    printTimingStatistics(current_operation_name, total_conversion_time_ms);
    std::cout << (all_ok ? GREEN_COLOR : RED_COLOR) << "文件转换阶段 " << (all_ok ? "全部成功" : "存在失败项") << "。" << RESET_COLOR << std::endl;
    return all_ok;
}

/**
 * @brief 阶段四：检验输出文件
 */
bool FilePipelineManager::validateOutputFiles(bool enable_day_count_check) {
    const std::string current_operation_name = "validateOutputFiles";
    std::cout << "\n--- 阶段: 检验输出文件 ---" << std::endl;
    if (source_to_output_map_.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 没有已转换的文件可供检验。请先运行转换操作。" << RESET_COLOR << std::endl;
        return true;
    }

    bool all_ok = true;
    double total_validation_time_ms = 0.0;

    for (const auto& pair : source_to_output_map_) {
        const auto& output_file = pair.second;
        AppOptions opts;
        opts.validate_output = true;
        opts.enable_day_count_check = enable_day_count_check;

        ProcessingResult result = processor_.processFile("", output_file, opts);
        total_validation_time_ms += result.timings.validation_output_ms;

        if (!result.success) {
            all_ok = false;
        }
    }

    printTimingStatistics(current_operation_name, total_validation_time_ms);
    std::cout << (all_ok ? GREEN_COLOR : RED_COLOR) << "输出文件检验阶段 " << (all_ok ? "全部通过" : "存在失败项") << "。" << RESET_COLOR << std::endl;
    return all_ok;
}

/**
 * @brief 打印操作的计时统计
 */
void FilePipelineManager::printTimingStatistics(const std::string& operation_name, double total_time_ms) const {
    double total_time_s = total_time_ms / 1000.0;
    std::cout << "--------------------------------------\n";
    std::cout << "Timing Statistics:\n";
    std::cout << operation_name << "\n\n";
    std::cout << "Total time: " << std::fixed << std::setprecision(4) << total_time_s
              << " seconds (" << total_time_ms << " ms)\n";
    std::cout << "--------------------------------------\n";
}