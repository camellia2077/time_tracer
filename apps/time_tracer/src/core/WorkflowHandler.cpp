// core/WorkflowHandler.cpp
#include "core/WorkflowHandler.hpp"
#include "core/file/PipelineManager.hpp" 
#include "importer/DataImporter.hpp"
#include "core/importer/FileImportReader.hpp"
#include "core/database/DBManager.hpp"
#include "common/AnsiColors.hpp"
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

WorkflowHandler::WorkflowHandler(const std::string& db_path, const AppConfig& config, const fs::path& output_root_path)
    : app_config_(config),
      db_path_(db_path),
      output_root_path_(output_root_path)
{}

WorkflowHandler::~WorkflowHandler() = default;

void WorkflowHandler::run_preprocessing(const std::string& input_path, const AppOptions& options) {
    std::cout << "\n--- 开始预处理流程 ---" << std::endl;
    PipelineManager pipeline(app_config_, output_root_path_);
    
    if (!pipeline.run(input_path, options)) {
        throw std::runtime_error("预处理流程执行失败，请检查上方错误日志。");
    }
    
    std::cout << GREEN_COLOR << "成功: 预处理流程执行完毕。" << RESET_COLOR << std::endl;
}

const AppConfig& WorkflowHandler::get_config() const {
    return app_config_;
}

void WorkflowHandler::run_database_import(const std::string& processed_path_str) {
    auto import_payload = FileImportReader::read_json_files(processed_path_str);

    if (import_payload.empty()) {
        std::cerr << RED_COLOR << "错误: 没有可导入的文件内容，导入终止。" << RESET_COLOR << std::endl;
        return;
    }

    DBManager db_manager(db_path_);
    db_manager.close_database(); 
    
    std::cout << "开始将数据导入数据库..." << std::endl;
    handle_import_json_content(db_path_, import_payload);
    std::cout << "导入过程结束。" << std::endl;
}

void WorkflowHandler::run_database_import_from_memory(const std::map<std::string, std::vector<InputData>>& data_map) {
    DBManager db_manager(db_path_);
    db_manager.close_database(); 
    std::cout << "开始从内存数据导入过程..." << std::endl;
    handle_process_memory_data(db_path_, data_map); 
    std::cout << "内存导入过程结束。" << std::endl;
}

void WorkflowHandler::run_full_pipeline_and_import(const std::string& source_path, DateCheckMode date_check_mode, bool save_processed) {
    std::cout << "\n--- 开始完整流水线处理 ---" << std::endl;
    
    DBManager db_manager(db_path_);
    db_manager.close_database();

    PipelineManager pipeline(app_config_, output_root_path_);
    
    AppOptions full_options;
    full_options.validate_source = true;
    full_options.convert = true;
    full_options.validate_output = true;
    full_options.date_check_mode = date_check_mode;
    full_options.save_processed_output = save_processed;

    auto result_context_opt = pipeline.run(source_path, full_options);

    if (result_context_opt) {
        const auto& context = *result_context_opt;

        // [修正] processed_data_memory -> result.processed_data
        if (!context.result.processed_data.empty()) {
            run_database_import_from_memory(context.result.processed_data);
            std::cout << GREEN_COLOR << "\n成功: 数据已通过内存直连导入数据库。" << RESET_COLOR << std::endl;
        } 
        // [修正] generated_files -> state.generated_files
        else if (save_processed && !context.state.generated_files.empty()) {
            std::cout << YELLOW_COLOR << "\n注意: 内存数据为空，尝试回退到文件导入..." << RESET_COLOR << std::endl;
            // [修正] output_root -> config.output_root
            run_database_import(context.config.output_root.string() + "/Processed_Date");
        }
        else {
             std::cout << YELLOW_COLOR << "\n警告: 没有产生可导入的数据。" << RESET_COLOR << std::endl;
        }

        if (save_processed) {
            std::cout << "输出文件已保存至: " << fs::absolute(output_root_path_) << std::endl;
        } else {
            std::cout << "提示: 程序从内存中读取转换后json结构并且插入数据库，未存储到本地。" << std::endl;
        }

    } else {
        std::cerr << RED_COLOR << "错误: 文件处理流水线失败，操作终止。" << RESET_COLOR << std::endl;
    }
}