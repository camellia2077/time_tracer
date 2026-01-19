#ifndef CORE_WORKFLOW_HANDLER_HPP_
#define CORE_WORKFLOW_HANDLER_HPP_

#include "core/interfaces/i_workflow_handler.hpp" // [新增]
#include <string>
#include <filesystem>
#include <map>
#include <vector>
#include "common/config/app_config.hpp"
#include "common/app_options.hpp"
#include "validator/common/ValidatorUtils.hpp"
#include "common/model/daily_log.hpp" 

namespace fs = std::filesystem;

// [修改] 继承 IWorkflowHandler
class WorkflowHandler : public IWorkflowHandler {
public:
    WorkflowHandler(const std::string& db_path, const AppConfig& config, const fs::path& output_root_path);
    ~WorkflowHandler() override; 

    void run_converter(const std::string& input_path, const AppOptions& options) override;
    void run_database_import(const std::string& processed_path_str) override;
    void run_database_import_from_memory(const std::map<std::string, std::vector<DailyLog>>& data_map) override;
    void run_ingest(const std::string& source_path, DateCheckMode date_check_mode, bool save_processed = false) override;
    const AppConfig& get_config() const override;

private:
    const AppConfig& app_config_;
    std::string db_path_;
    fs::path output_root_path_;
};

#endif // CORE_WORKFLOW_HANDLER_HPP_