// action_handler/FileProcessingHandler.cpp
#include "action_handler/FileProcessingHandler.hpp"
#include "action_handler/file/FilePipelineManager.hpp" 
#include "db_inserter/DataImporter.hpp"
#include "common/AnsiColors.hpp"
#include "action_handler/database/DatabaseManager.hpp"

#include <iostream>
#include <optional>
#include <stdexcept>

namespace fs = std::filesystem;

FileProcessingHandler::FileProcessingHandler(const std::string& db_path, const AppConfig& config, const std::string& main_config_path, const fs::path& output_root_path)
    : app_config_(config),
      main_config_path_(main_config_path),
      db_path_(db_path),
      output_root_path_(output_root_path)
{
}

FileProcessingHandler::~FileProcessingHandler() = default;

void FileProcessingHandler::run_preprocessing(const std::string& input_path, const AppOptions& options) {
    std::cout << "\n--- 开始预处理流程 ---" << std::endl;
    FilePipelineManager pipeline(app_config_, output_root_path_);
    
    std::string extension_to_collect = ".txt";
    if (options.validate_output && !options.convert && !options.validate_source) {
        extension_to_collect = ".json";
    }

    if (!pipeline.collectFiles(input_path, extension_to_collect)) {
         throw std::runtime_error("无法从指定路径收集 " + extension_to_collect + " 文件，操作中止。");
    }

    if (options.validate_source && !pipeline.validateSourceFiles()) {
        throw std::runtime_error("源文件验证失败。");
    }
    if (options.convert && !pipeline.convertFiles()) { // Assuming convertFiles doesn't need a path
        throw std::runtime_error("文件转换失败。");
    }
    if (options.validate_output && !pipeline.validateOutputFiles(options.enable_day_count_check)) {
        throw std::runtime_error("输出文件验证失败。");
    }
    
    std::cout << GREEN_COLOR << "成功: 预处理流程执行完毕。" << RESET_COLOR << std::endl;
}

const AppConfig& FileProcessingHandler::get_config() const {
    return app_config_;
}

void FileProcessingHandler::run_database_import(const std::string& processed_path_str) {
    fs::path processed_path(processed_path_str);

    if (!fs::exists(processed_path)) {
        std::cerr << RED_COLOR << "错误: " << RESET_COLOR << "路径 " << processed_path_str << " 不存在。导入中止。" << std::endl;
        return;
    }
    
    DatabaseManager db_manager(db_path_);
    db_manager.close_database(); 

    std::cout << "开始导入过程..." << std::endl;
    handle_process_files(db_path_, processed_path.string(), main_config_path_);
    std::cout << "导入过程结束。" << std::endl;
}

void FileProcessingHandler::run_full_pipeline_and_import(const std::string& source_path) {
    std::cout << "\n--- 开始完整流水线处理 ---" << std::endl;
    
    DatabaseManager db_manager(db_path_);
    db_manager.close_database();

    FilePipelineManager pipeline(app_config_, output_root_path_);
    
    if (auto processed_path = pipeline.run(source_path)) {
        run_database_import(processed_path->string());
        std::cout << GREEN_COLOR << "\n成功: 完整流水线处理完毕并已导入数据。" << RESET_COLOR << std::endl;
        std::cout << "所有输出文件位于: " << fs::absolute(output_root_path_) << std::endl;
    } else {
        std::cerr << RED_COLOR << "错误: 文件处理流水线失败，操作终止。" << RESET_COLOR << std::endl;
    }
}