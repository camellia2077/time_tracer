// core/WorkflowHandler.cpp
#include "core/WorkflowHandler.hpp"
#include "core/file/PipelineManager.hpp" 
#include "db_inserter/DataImporter.hpp"
#include "common/AnsiColors.hpp"
#include "core/database/DBManager.hpp"
#include "file_handler/FileUtils.hpp"
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
    
    // pipeline.run 现在返回 std::optional<PipelineContext>，它在布尔上下文中仍然有效
    if (!pipeline.run(input_path, options)) {
        throw std::runtime_error("预处理流程执行失败，请检查上方错误日志。");
    }
    
    std::cout << GREEN_COLOR << "成功: 预处理流程执行完毕。" << RESET_COLOR << std::endl;
}

const AppConfig& WorkflowHandler::get_config() const {
    return app_config_;
}

// 传统的基于文件的导入
void WorkflowHandler::run_database_import(const std::string& processed_path_str) {
    // 1. 准备文件列表
    std::cout << "正在扫描待导入文件..." << std::endl;
    std::vector<std::string> input_paths = {processed_path_str};
    std::vector<std::string> json_files = FileUtils::resolve_files(input_paths, ".json");

    if (json_files.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 在路径 " << processed_path_str << " 下未找到 .json 文件。" << RESET_COLOR << std::endl;
        return;
    }

    // 2. 准备数据库
    DBManager db_manager(db_path_);
    db_manager.close_database(); 
    
    std::cout << "开始从文件导入过程 (共 " << json_files.size() << " 个文件)..." << std::endl;
    
    // 3. 调用 DataImporter (传入已解析的文件列表)
    handle_process_files(db_path_, json_files);
    
    std::cout << "文件导入过程结束。" << std::endl;
}

// 基于内存数据的导入
void WorkflowHandler::run_database_import_from_memory(const std::map<std::string, std::vector<InputData>>& data_map) {
    DBManager db_manager(db_path_);
    db_manager.close_database(); 
    std::cout << "开始从内存数据导入过程..." << std::endl;
    
    // [注意] 此函数需要在 DataImporter.hpp 中声明并实现
    // handle_process_files(db_path_, processed_path.string()); // 旧的
    handle_process_memory_data(db_path_, data_map); // [新接口]
    
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

        // [核心修改] 优先尝试从内存导入
        if (!context.processed_data_memory.empty()) {
            run_database_import_from_memory(context.processed_data_memory);
            std::cout << GREEN_COLOR << "\n成功: 数据已通过内存直连导入数据库。" << RESET_COLOR << std::endl;
        } 
        // 如果内存为空但文件已保存（极其罕见的情况），回退到文件导入
        else if (save_processed && !context.generated_files.empty()) {
            std::cout << YELLOW_COLOR << "\n注意: 内存数据为空，尝试回退到文件导入..." << RESET_COLOR << std::endl;
            run_database_import(context.output_root.string() + "/Processed_Date");
        }
        else {
             std::cout << YELLOW_COLOR << "\n警告: 没有产生可导入的数据。" << RESET_COLOR << std::endl;
        }

        // 提示文件保存位置
        if (save_processed) {
            std::cout << "输出文件已保存至: " << fs::absolute(output_root_path_) << std::endl;
        } else {
            std::cout << "提示: 程序从内存中读取转换后json结构并且插入数据库，未存储到本地。" << std::endl;
        }

    } else {
        std::cerr << RED_COLOR << "错误: 文件处理流水线失败，操作终止。" << RESET_COLOR << std::endl;
    }
}