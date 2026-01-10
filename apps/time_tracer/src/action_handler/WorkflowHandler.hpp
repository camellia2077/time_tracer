// action_handler/WorkflowHandler.hpp
#ifndef WORKFLOW_HANDLER_HPP
#define WORKFLOW_HANDLER_HPP

#include <string>
#include <filesystem>
#include "common/AppConfig.hpp" 
#include "reprocessing/validator/common/ValidatorUtils.hpp"

namespace fs = std::filesystem;

class WorkflowHandler { // [修改] 类名变更
public:
    // [修改] 构造函数名变更
    WorkflowHandler(const std::string& db_path, const AppConfig& config, const fs::path& output_root_path);
    ~WorkflowHandler();

    void run_preprocessing(const std::string& input_path, const AppOptions& options);
    void run_database_import(const std::string& processed_path_str);
    
    void run_full_pipeline_and_import(const std::string& source_path, DateCheckMode date_check_mode);
    
    const AppConfig& get_config() const;

private:
    const AppConfig& app_config_;
    std::string db_path_;
    fs::path output_root_path_;
};

#endif // WORKFLOW_HANDLER_HPP