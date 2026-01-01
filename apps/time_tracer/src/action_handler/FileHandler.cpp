// action_handler/FileHandler.cpp
#include "action_handler/FileHandler.hpp"
#include "action_handler/file/PipelineManager.hpp" 
#include "db_inserter/DataImporter.hpp"
#include "common/AnsiColors.hpp"
#include "action_handler/database/DBManager.hpp"
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

FileHandler::FileHandler(const std::string& db_path, const AppConfig& config, const std::string& main_config_path, const fs::path& output_root_path)
    : app_config_(config),
      main_config_path_(main_config_path),
      db_path_(db_path),
      output_root_path_(output_root_path)
{}

FileHandler::~FileHandler() = default;

void FileHandler::run_preprocessing(const std::string& input_path, const AppOptions& options) {
    std::cout << "\n--- 开始预处理流程 ---" << std::endl;
    PipelineManager pipeline(app_config_, output_root_path_);
    
    // [修复] 不再手动分步调用，而是直接调用统一的 run 接口
    if (!pipeline.run(input_path, options)) {
        throw std::runtime_error("预处理流程执行失败，请检查上方错误日志。");
    }
    
    std::cout << GREEN_COLOR << "成功: 预处理流程执行完毕。" << RESET_COLOR << std::endl;
}

const AppConfig& FileHandler::get_config() const {
    return app_config_;
}

void FileHandler::run_database_import(const std::string& processed_path_str) {
    // ... (保持不变) ...
    fs::path processed_path(processed_path_str);
    if (!fs::exists(processed_path)) {
        std::cerr << RED_COLOR << "错误: " << RESET_COLOR << "路径 " << processed_path_str << " 不存在。导入中止。" << std::endl;
        return;
    }
    DBManager db_manager(db_path_);
    db_manager.close_database(); 
    std::cout << "开始导入过程..." << std::endl;
    handle_process_files(db_path_, processed_path.string());
    std::cout << "导入过程结束。" << std::endl;
}

void FileHandler::run_full_pipeline_and_import(const std::string& source_path, DateCheckMode date_check_mode) {
    std::cout << "\n--- 开始完整流水线处理 ---" << std::endl;
    
    DBManager db_manager(db_path_);
    db_manager.close_database();

    PipelineManager pipeline(app_config_, output_root_path_);
    
    // [修复] 构造一个默认全开启的 options 用于完整流程
    AppOptions full_options;
    full_options.validate_source = true;
    full_options.convert = true;
    full_options.validate_output = true;
    full_options.date_check_mode = date_check_mode;

    if (auto processed_path = pipeline.run(source_path, full_options)) {
        run_database_import(processed_path->string());
        std::cout << GREEN_COLOR << "\n成功: 完整流水线处理完毕并已导入数据。" << RESET_COLOR << std::endl;
        std::cout << "所有输出文件位于: " << fs::absolute(output_root_path_) << std::endl;
    } else {
        std::cerr << RED_COLOR << "错误: 文件处理流水线失败，操作终止。" << RESET_COLOR << std::endl;
    }
}