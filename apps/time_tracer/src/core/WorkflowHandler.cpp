// core/WorkflowHandler.cpp
#include "core/WorkflowHandler.hpp"
#include "core/file/PipelineManager.hpp" 
#include "importer/DataImporter.hpp"
#include "common/AnsiColors.hpp"
#include "core/database/DBManager.hpp"
#include "io/utils/FileUtils.hpp"
#include "io/core/FileReader.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <utility>

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

// [核心重构] 传统的基于文件的导入
// 现在由 Core 负责读取 IO，DB Inserter 只负责处理数据
void WorkflowHandler::run_database_import(const std::string& processed_path_str) {
    // 1. 准备文件列表
    std::cout << "正在扫描待导入文件..." << std::endl;
    std::vector<std::string> input_paths = {processed_path_str};
    std::vector<std::string> json_files = FileUtils::resolve_files(input_paths, ".json");

    if (json_files.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 在路径 " << processed_path_str << " 下未找到 .json 文件。" << RESET_COLOR << std::endl;
        return;
    }

    // 2. [新增步骤] 使用 IO 模块读取所有文件内容
    std::cout << "正在读取 " << json_files.size() << " 个文件的内容..." << std::endl;
    std::vector<std::pair<std::string, std::string>> import_payload;
    import_payload.reserve(json_files.size());

    int read_failure_count = 0;
    for (const auto& file_path : json_files) {
        try {
            // 使用 io 模块读取
            std::string content = FileReader::read_content(file_path);
            // 记录文件名作为标识符 (context_name)
            import_payload.push_back({file_path, std::move(content)});
        } catch (const std::exception& e) {
            std::cerr << RED_COLOR << "读取失败: " << file_path << " - " << e.what() << RESET_COLOR << std::endl;
            read_failure_count++;
        }
    }

    if (import_payload.empty()) {
        std::cerr << RED_COLOR << "错误: 没有成功读取到任何文件内容，导入终止。" << RESET_COLOR << std::endl;
        return;
    }

    if (read_failure_count > 0) {
        std::cout << YELLOW_COLOR << "警告: 有 " << read_failure_count << " 个文件读取失败，将跳过这些文件。" << RESET_COLOR << std::endl;
    }

    // 3. 准备数据库
    DBManager db_manager(db_path_);
    db_manager.close_database(); 
    
    std::cout << "开始将数据导入数据库..." << std::endl;
    
    // 4. [修改] 调用 DataImporter 的新接口 (传入内容列表)
    handle_import_json_content(db_path_, import_payload);
    
    std::cout << "导入过程结束。" << std::endl;
}

// 基于内存数据的导入
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

    // 获取执行结果上下文
    auto result_context_opt = pipeline.run(source_path, full_options);

    if (result_context_opt) {
        const auto& context = *result_context_opt;

        // 优先尝试从内存导入 (最高效路径)
        if (!context.processed_data_memory.empty()) {
            run_database_import_from_memory(context.processed_data_memory);
            std::cout << GREEN_COLOR << "\n成功: 数据已通过内存直连导入数据库。" << RESET_COLOR << std::endl;
        } 
        // 内存为空但文件已保存 (回退路径)
        else if (save_processed && !context.generated_files.empty()) {
            std::cout << YELLOW_COLOR << "\n注意: 内存数据为空，尝试回退到文件导入..." << RESET_COLOR << std::endl;
            // 这里会调用上面重构过的 run_database_import
            run_database_import(context.output_root.string() + "/Processed_Date");
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