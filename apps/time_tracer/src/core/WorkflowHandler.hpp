// core/WorkflowHandler.hpp
#ifndef WORKFLOW_HANDLER_HPP
#define WORKFLOW_HANDLER_HPP

#include <string>
#include <filesystem>
#include <map>
#include <vector>
#include "common/AppConfig.hpp" 
#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include "reprocessing/converter/model/InputData.hpp" // [新增]

namespace fs = std::filesystem;

class WorkflowHandler {
public:
    WorkflowHandler(const std::string& db_path, const AppConfig& config, const fs::path& output_root_path);
    ~WorkflowHandler();

    void run_preprocessing(const std::string& input_path, const AppOptions& options);
    
    // 传统的基于文件的导入 (保留用于 import 命令)
    void run_database_import(const std::string& processed_path_str);

    // [新增] 基于内存数据的导入 (用于 run-pipeline/blink)
    void run_database_import_from_memory(const std::map<std::string, std::vector<InputData>>& data_map);
    
    void run_full_pipeline_and_import(const std::string& source_path, DateCheckMode date_check_mode, bool save_processed = false);
    
    const AppConfig& get_config() const;

private:
    const AppConfig& app_config_;
    std::string db_path_;
    fs::path output_root_path_;
};

#endif // WORKFLOW_HANDLER_HPP