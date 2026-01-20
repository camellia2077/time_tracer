// core/workflow_handler.cpp
#include "core/workflow_handler.hpp"
#include "core/pipeline/pipeline_manager.hpp" 
#include "importer/data_importer.hpp"         // 包含 handle_process_memory_data
#include "core/helpers/file_import_reader.hpp"
#include "core/database/db_manager.hpp"
#include "serializer/json_serializer.hpp"     // 包含 JsonSerializer

#include "common/ansi_colors.hpp"
#include <iostream>
#include <stdexcept>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

// 使用之前定义的命名空间
using namespace core::pipeline;
using namespace core::helpers;

WorkflowHandler::WorkflowHandler(const std::string& db_path, const AppConfig& config, const fs::path& output_root_path)
    : app_config_(config),
      db_path_(db_path),
      output_root_path_(output_root_path)
{}

WorkflowHandler::~WorkflowHandler() = default;

void WorkflowHandler::run_converter(const std::string& input_path, const AppOptions& options) {
    PipelineManager pipeline(app_config_, output_root_path_);
    if (!pipeline.run(input_path, options)) {
        throw std::runtime_error("Converter Pipeline Failed.");
    }
}

const AppConfig& WorkflowHandler::get_config() const {
    return app_config_;
}

// [关键修复] 重写此函数：读取文件 -> 解析 JSON -> 传递 Struct 给 Importer
void WorkflowHandler::run_database_import(const std::string& processed_path_str) {
    // 1. 使用 Helper 读取文件内容
    auto import_payload = FileImportReader::read_json_files(processed_path_str);
    
    if (import_payload.empty()) return;

    std::cout << "正在解析 JSON 数据..." << std::endl;

    // 2. 在 Core 层完成解析 (JSON string -> DailyLog Struct)
    std::map<std::string, std::vector<DailyLog>> memory_data;

    for(const auto& [filepath, content] : import_payload) {
        try {
            auto json_obj = nlohmann::json::parse(content);
            std::vector<DailyLog> logs = serializer::JsonSerializer::deserializeDays(json_obj);
            
            // 使用文件路径作为 key (仅用于日志/批次区分)
            memory_data[filepath] = logs;
            
        } catch (const std::exception& e) {
            std::cerr << RED_COLOR << "解析文件失败 " << filepath << ": " << e.what() << RESET_COLOR << std::endl;
        }
    }

    if (memory_data.empty()) {
        std::cout << YELLOW_COLOR << "没有有效的 JSON 数据可供导入。" << RESET_COLOR << std::endl;
        return;
    }

    // 3. 调用内存导入接口 (Importer 不再依赖 JSON)
    run_database_import_from_memory(memory_data);
}

void WorkflowHandler::run_database_import_from_memory(const std::map<std::string, std::vector<DailyLog>>& data_map) {
     handle_process_memory_data(db_path_, data_map);
}

void WorkflowHandler::run_ingest(const std::string& source_path, DateCheckMode date_check_mode, bool save_processed) {
    std::cout << "\n--- 启动数据摄入 (Ingest) ---" << std::endl;
    
    // RAII 检查数据库连接
    {
        DBManager db_manager(db_path_);
    } 

    // 这里依然使用 PipelineManager，因为这是内部实现机制，没问题
    PipelineManager pipeline(app_config_, output_root_path_);
    
    AppOptions full_options;
    full_options.validate_structure = true;
    full_options.convert = true;
    full_options.validate_logic = true;
    full_options.date_check_mode = date_check_mode;
    full_options.save_processed_output = save_processed;

    auto result_context_opt = pipeline.run(source_path, full_options);

    if (result_context_opt) {
        const auto& context = *result_context_opt;

        if (!context.result.processed_data.empty()) {
            std::cout << "\n--- 流水线验证通过，准备入库 ---" << std::endl;
            run_database_import_from_memory(context.result.processed_data);
            std::cout << GREEN_COLOR << "\n=== Ingest 执行成功 ===" << RESET_COLOR << std::endl;
        } else {
             std::cout << YELLOW_COLOR << "\n=== Ingest 完成但无数据产生 ===" << RESET_COLOR << std::endl;
        }
    } else {
        std::cerr << RED_COLOR << "\n=== Ingest 执行失败 ===" << RESET_COLOR << std::endl;
        throw std::runtime_error("Ingestion process failed.");
    }
}