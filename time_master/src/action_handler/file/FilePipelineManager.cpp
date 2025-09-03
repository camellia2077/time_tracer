// file: action_handler/file/FilePipelineManager.cpp


#include "FilePipelineManager.hpp"
#include "common/common_utils.hpp"
#include "file_handler/FileUtils.hpp"
#include <iostream>
#include <iomanip>

namespace fs = std::filesystem;

/**
 * @brief 构造函数实现
 */
FilePipelineManager::FilePipelineManager(const AppConfig& config, const fs::path& output_root)
    : app_config_(config), processor_(config), output_root_(output_root) {}

/**
 * @brief 运行完整的处理流水线
 */
std::optional<fs::path> FilePipelineManager::run(const std::string& input_path) {
    fs::path input_root_path(input_path);
    if (!collectFiles(input_path) || !validateSourceFiles() || !convertFiles() || !validateOutputFiles(true)) {
        return std::nullopt; // 如果任何一个阶段失败，则返回空
    }

    // 根据输入是目录还是文件，返回正确的处理后路径
    if (fs::is_directory(input_root_path)) {
        return output_root_ / ("Processed_" + input_root_path.filename().string());
    }
    return output_root_;
}

/**
 * @brief 阶段一：收集需要处理的文件
 */
// [核心修改] 更新函数定义以接收 extension 参数
bool FilePipelineManager::collectFiles(const std::string& input_path, const std::string& extension) {
    input_root_ = fs::path(input_path);
    if (!fs::exists(input_root_)) {
        std::cerr << RED_COLOR << "错误: 输入的路径不存在: " << input_path << RESET_COLOR << std::endl;
        return false;
    }

    files_to_process_.clear();
    source_to_output_map_.clear();

    // [核心修改] 使用传入的 extension 参数来查找文件
    files_to_process_ = FileUtils::find_files_by_extension_recursively(input_root_, extension);

    // 如果输入本身就是一个符合条件的文件，也将其加入列表
    if (fs::is_regular_file(input_root_) && input_root_.extension() == extension) {
        // 避免重复添加
        if (std::find(files_to_process_.begin(), files_to_process_.end(), input_root_) == files_to_process_.end()) {
            files_to_process_.push_back(input_root_);
        }
    }

    std::cout << "信息: 成功收集到 " << files_to_process_.size() << " 个待处理文件 (" << extension << ")." << std::endl;
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
    fs::path processed_output_dir = output_root_; // 默认输出目录

    if (is_dir) {
        // 如果是目录，则在输出根目录下创建一个 "Processed_..." 子目录
        processed_output_dir /= ("Processed_" + input_root_.filename().string());
    }
    fs::create_directories(processed_output_dir);

    bool all_ok = true;
    double total_conversion_time_ms = 0.0;

    for (const auto& file : files_to_process_) {
        fs::path output_file_path;
        if (is_dir) { // 如果是目录，保持原有的相对路径结构
            output_file_path = processed_output_dir / fs::relative(file, input_root_);
            output_file_path.replace_extension(".json");
            fs::create_directories(output_file_path.parent_path());
        } else {
            // 如果是单个文件，直接在输出目录中创建
            output_file_path = processed_output_dir / file.filename();
            output_file_path.replace_extension(".json");
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

    // 创建一个统一的列表来存放待检验的文件
    std::vector<fs::path> files_to_validate;

    if (!source_to_output_map_.empty()) {
        // 场景1: 已执行转换，从 map 中提取输出文件
        for (const auto& pair : source_to_output_map_) {
            files_to_validate.push_back(pair.second);
        }
    } else if (!files_to_process_.empty()) {
        // 场景2: 未执行转换，直接使用最初收集的输入文件
        std::cout << "信息: 未执行转换，将直接检验输入文件。" << std::endl;
        files_to_validate = files_to_process_;
    }

    // 检查最终确定的列表是否为空
    if (files_to_validate.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 没有文件可供检验。" << RESET_COLOR << std::endl;
        return true;
    }

    bool all_ok = true;
    double total_validation_time_ms = 0.0;

    // 循环遍历新的统一列表
    for (const auto& file_to_check : files_to_validate) {
        AppOptions opts;
        opts.validate_output = true;
        opts.enable_day_count_check = enable_day_count_check;

        // 第一个参数是源文件路径，在仅检验输出时不需要，传空字符串即可
        ProcessingResult result = processor_.processFile("", file_to_check, opts);
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