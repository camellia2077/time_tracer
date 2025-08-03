#include "common/pch.h"
#include "action_handler/FileProcessingHandler.h"
#include "action_handler/file/FilePipelineManager.h"
#include "db_inserter/DataImporter.h"
#include "common/common_utils.h"
#include "action_handler/database/DatabaseManager.h"
#include <iostream>
#include <optional>

namespace fs = std::filesystem;

FileProcessingHandler::FileProcessingHandler(const std::string& db_name, const AppConfig& config, const std::string& main_config_path)
    : app_config_(config),
      main_config_path_(main_config_path),
      db_name_(db_name)
{
}

FileProcessingHandler::~FileProcessingHandler() = default;

const AppConfig& FileProcessingHandler::get_config() const {
    return app_config_;
}

void FileProcessingHandler::run_database_import(const std::string& processed_path_str) {
    fs::path processed_path(processed_path_str);
    if (!fs::exists(processed_path) || !fs::is_directory(processed_path)) {
        std::cerr << RED_COLOR << "错误: " << RESET_COLOR << "路径不存在或不是目录。导入中止。" << std::endl;
        return;
    }
    
    DatabaseManager db_manager(db_name_);
    db_manager.close_database();

    std::cout << "开始导入过程..." << std::endl;
    handle_process_files(db_name_, processed_path.string(), main_config_path_);
    std::cout << "导入过程结束。" << std::endl;
}

void FileProcessingHandler::run_full_pipeline_and_import(const std::string& source_path) {
    std::cout << "\n--- 开始完整流水线处理 ---" << std::endl;
    DatabaseManager db_manager(db_name_);
    db_manager.close_database();

    FilePipelineManager pipeline(app_config_);
    std::optional<fs::path> output_path = pipeline.run(source_path);

    if (output_path) {
        run_database_import(output_path->string());
        std::cout << GREEN_COLOR << "\n成功: 完整流水线处理完毕并已导入数据。" << RESET_COLOR << std::endl;
    } else {
        std::cerr << RED_COLOR << "错误: 文件处理流水线失败，操作终止。" << RESET_COLOR << std::endl;
    }
}