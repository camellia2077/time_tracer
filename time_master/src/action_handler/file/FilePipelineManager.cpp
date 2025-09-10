// action_handler/file/FilePipelineManager.cpp
#include "FilePipelineManager.hpp"
#include "common/AnsiColors.hpp"
#include "file_handler/FileUtils.hpp"
#include "reprocessing/converter/pipelines/OutputGenerator.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <map>

// --- [核心修正] 新增缺失的头文件 ---
#include "reprocessing/validator/FileValidator.hpp"
#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include "reprocessing/converter/ConverterConfig.hpp"


namespace fs = std::filesystem;

FilePipelineManager::FilePipelineManager(const AppConfig& config, const fs::path& output_root)
    : app_config_(config), processor_(config), output_root_(output_root) {}

std::optional<fs::path> FilePipelineManager::run(const std::string& input_path) {
    fs::path input_root_path(input_path);

    if (!collectFiles(input_path) || !validateSourceFiles() || !convertFiles() || !validateOutputFiles(true)) {
        return std::nullopt;
    }

    // 成功后返回输出文件的根目录
    return output_root_ / "Processed_Date";
}

bool FilePipelineManager::collectFiles(const std::string& input_path, const std::string& extension) {
    input_root_ = fs::path(input_path);
    if (!fs::exists(input_root_)) {
        std::cerr << RED_COLOR << "错误: 输入的路径不存在: " << input_path << RESET_COLOR << std::endl;
        return false;
    }
    files_to_process_.clear();
    generated_output_files_.clear();
    files_to_process_ = FileUtils::find_files_by_extension_recursively(input_root_, extension);

    if (fs::is_regular_file(input_root_) && input_root_.extension() == extension) {
        if (std::find(files_to_process_.begin(), files_to_process_.end(), input_root_) == files_to_process_.end()) {
            files_to_process_.push_back(input_root_);
        }
    }
    if (files_to_process_.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 在指定路径下没有找到 " << extension << " 文件。" << RESET_COLOR << std::endl;
        return false;
    }
    std::cout << "信息: 成功收集到 " << files_to_process_.size() << " 个待处理文件 (" << extension << ")." << std::endl;
    return true;
}

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
        ProcessingResult result = processor_.processFile(file, opts);
        total_validation_time_ms += result.timings.validation_source_ms;
        if (!result.success) all_ok = false;
    }
    printTimingStatistics(current_operation_name, total_validation_time_ms);
    std::cout << (all_ok ? GREEN_COLOR : RED_COLOR) << "源文件检验阶段 " << (all_ok ? "全部通过" : "存在失败项") << "。" << RESET_COLOR << std::endl;
    return all_ok;
}

bool FilePipelineManager::convertFiles() {
    const std::string current_operation_name = "convertFiles";
    std::cout << "\n--- 阶段: 转换文件 ---" << std::endl;
    if (files_to_process_.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 没有已收集的文件可供转换。" << RESET_COLOR << std::endl;
        return false;
    }

    auto start_time = std::chrono::steady_clock::now();

    std::stringstream combined_stream;
    for (const auto& source_file : files_to_process_) {
        std::ifstream file_stream(source_file);
        if (file_stream) {
            combined_stream << file_stream.rdbuf();
        }
    }
    
    std::vector<InputData> all_processed_days = processor_.convertStreamToData(combined_stream);
    if (all_processed_days.empty()) {
        std::cerr << RED_COLOR << "文件转换失败，没有生成任何数据。" << RESET_COLOR << std::endl;
        return false;
    }

    std::map<std::string, std::vector<InputData>> monthly_data;
    for (const auto& day : all_processed_days) {
        if (day.date.length() >= 6) {
            std::string year_month = day.date.substr(0, 4) + "_" + day.date.substr(4, 2);
            monthly_data[year_month].push_back(day);
        }
    }

    // --- [核心修正] ---
    // 在此作用域内创建并加载 ConverterConfig
    ConverterConfig converter_config;
    if (!converter_config.load(app_config_.interval_processor_config_path)) {
        std::cerr << RED_COLOR << "错误: 无法加载转换器配置，文件写入中止。" << RESET_COLOR << std::endl;
        return false;
    }
    
    OutputGenerator generator;
    fs::path output_dir_base = output_root_ / "Processed_Date";

    for (const auto& pair : monthly_data) {
        const std::string& year_month = pair.first;
        const std::vector<InputData>& month_days = pair.second;
        
        std::string year = year_month.substr(0, 4);
        fs::path month_output_dir = output_dir_base / year;
        fs::create_directories(month_output_dir);

        fs::path output_file_path = month_output_dir / (year_month + ".json");
        std::ofstream outFile(output_file_path);
        if (!outFile.is_open()) {
            std::cerr << RED_COLOR << "错误: 无法创建输出文件: " << output_file_path << RESET_COLOR << std::endl;
            continue;
        }
        
        // --- [核心修正] ---
        // 传递正确加载的 converter_config 对象
        generator.write(outFile, month_days, converter_config); 
        generated_output_files_.push_back(output_file_path);
    }
    
    auto end_time = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> duration = end_time - start_time;
    printTimingStatistics(current_operation_name, duration.count());
    
    std::cout << GREEN_COLOR << "文件转换阶段 全部成功。" << RESET_COLOR << std::endl;
    return true;
}

bool FilePipelineManager::validateOutputFiles(bool enable_day_count_check) {
    const std::string current_operation_name = "validateOutputFiles";
    std::cout << "\n--- 阶段: 检验输出文件 ---" << std::endl;
    if (generated_output_files_.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 没有转换后的文件可供检验。" << RESET_COLOR << std::endl;
        return true;
    }
    
    bool all_ok = true;
    double total_validation_time_ms = 0.0;
    FileValidator output_validator(app_config_.interval_processor_config_path);

    for (const auto& file_to_validate : generated_output_files_) {
        auto start_time = std::chrono::steady_clock::now();
        std::set<Error> errors;
        if (!output_validator.validate(file_to_validate.string(), ValidatorType::JsonOutput, errors, enable_day_count_check)) {
            printGroupedErrors(file_to_validate.string(), errors);
            all_ok = false;
        }
        auto end_time = std::chrono::steady_clock::now();
        total_validation_time_ms += std::chrono::duration<double, std::milli>(end_time - start_time).count();
    }

    printTimingStatistics(current_operation_name, total_validation_time_ms);
    std::cout << (all_ok ? GREEN_COLOR : RED_COLOR) << "输出文件检验阶段 " << (all_ok ? "全部通过" : "存在失败项") << "。" << RESET_COLOR << std::endl;
    return all_ok;
}

void FilePipelineManager::printTimingStatistics(const std::string& operation_name, double total_time_ms) const {
    double total_time_s = total_time_ms / 1000.0;
    std::cout << "--------------------------------------\n";
    std::cout << "Timing Statistics for: " << operation_name << "\n\n";
    std::cout << "Total time: " << std::fixed << std::setprecision(4) << total_time_s
              << " seconds (" << total_time_ms << " ms)\n";
    std::cout << "--------------------------------------\n";
}