// core/WorkflowHandler.cpp
#include "core/WorkflowHandler.hpp"
#include "core/file/PipelineManager.hpp" 
#include "importer/DataImporter.hpp"
#include "core/importer/FileImportReader.hpp"
#include "core/database/DBManager.hpp"
#include "serializer/JsonSerializer.hpp"
#include "common/AnsiColors.hpp"
#include <iostream>
#include <stdexcept>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

WorkflowHandler::WorkflowHandler(const std::string& db_path, const AppConfig& config, const fs::path& output_root_path)
    : app_config_(config),
      db_path_(db_path),
      output_root_path_(output_root_path)
{}

WorkflowHandler::~WorkflowHandler() = default;

// 仅转换模式
void WorkflowHandler::run_converter(const std::string& input_path, const AppOptions& options) {
    PipelineManager pipeline(app_config_, output_root_path_);
    if (!pipeline.run(input_path, options)) {
        throw std::runtime_error("Converter Pipeline Failed.");
    }
}

const AppConfig& WorkflowHandler::get_config() const {
    return app_config_;
}

// 仅导入模式 (Import 命令)
void WorkflowHandler::run_database_import(const std::string& processed_path_str) {
    auto import_payload = FileImportReader::read_json_files(processed_path_str);
    if (import_payload.empty()) {
        std::cerr << RED_COLOR << "错误: 没有找到可导入的 JSON 文件。" << RESET_COLOR << std::endl;
        return;
    }

    std::map<std::string, std::vector<DailyLog>> grouped_data;
    for (const auto& [filename, content] : import_payload) {
        try {
            auto json_obj = nlohmann::json::parse(content);
            std::vector<DailyLog> logs = serializer::JsonSerializer::deserializeDays(json_obj);
            for (auto& log : logs) {
                if (log.date.length() >= 7) {
                     std::string month = log.date.substr(0, 7);
                     grouped_data[month].push_back(std::move(log));
                }
            }
        } catch (const std::exception& e) {
             std::cerr << RED_COLOR << "解析失败: " << filename << " - " << e.what() << RESET_COLOR << std::endl;
        }
    }

    run_database_import_from_memory(grouped_data);
}

void WorkflowHandler::run_database_import_from_memory(const std::map<std::string, std::vector<DailyLog>>& data_map) {
    DBManager db_manager(db_path_);
    db_manager.close_database(); 
    handle_process_memory_data(db_path_, data_map); 
}

// [核心] 全流程模式 (Blink/Run 命令)
void WorkflowHandler::run_full_pipeline_and_import(const std::string& source_path, DateCheckMode date_check_mode, bool save_processed) {
    std::cout << "\n--- 启动全流程流水线 (Blink) ---" << std::endl;
    
    // 确保数据库就绪
    DBManager db_manager(db_path_);
    db_manager.close_database();

    PipelineManager pipeline(app_config_, output_root_path_);
    
    // 配置全开启
    AppOptions full_options;
    full_options.validate_source = true; // Step 1
    full_options.convert = true;         // Step 2
    full_options.validate_output = true; // Step 3
    full_options.date_check_mode = date_check_mode;
    full_options.save_processed_output = save_processed; // Step 4 (如果上面都成功)

    // 执行流水线
    auto result_context_opt = pipeline.run(source_path, full_options);

    // Step 5: 入库
    if (result_context_opt) {
        const auto& context = *result_context_opt;

        if (!context.result.processed_data.empty()) {
            std::cout << "\n--- 流水线验证通过，准备入库 ---" << std::endl;
            // 直接使用内存中的 Struct 数据，无需从磁盘重读
            run_database_import_from_memory(context.result.processed_data);
            std::cout << GREEN_COLOR << "\n=== 全流程执行成功 ===" << RESET_COLOR << std::endl;
        } else {
             std::cout << YELLOW_COLOR << "警告: 流水线通过但未产生有效数据。" << RESET_COLOR << std::endl;
        }
    } else {
        // 失败分支 (PipelineManager 内部已经打印了具体的错误步骤)
        std::cerr << RED_COLOR << "\n=== 全流程失败: 数据存在错误，已拦截入库操作 ===" << RESET_COLOR << std::endl;
    }
}