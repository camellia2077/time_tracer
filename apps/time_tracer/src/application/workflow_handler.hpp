// application/workflow_handler.hpp
#ifndef APPLICATION_WORKFLOW_HANDLER_H_
#define APPLICATION_WORKFLOW_HANDLER_H_

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "application/interfaces/i_workflow_handler.hpp"  // [新增]
#include "common/app_options.hpp"
#include "common/config/app_config.hpp"
#include "domain/model/daily_log.hpp"
#include "validator/common/ValidatorUtils.hpp"

namespace fs = std::filesystem;

// [修改] 继承 IWorkflowHandler
class WorkflowHandler : public IWorkflowHandler {
 public:
  WorkflowHandler(std::string db_path, const AppConfig& config,
                  fs::path output_root_path);
  ~WorkflowHandler() override;

  void run_converter(const std::string& input_path,
                     const AppOptions& options) override;
  void run_database_import(const std::string& processed_path_str) override;
  void run_database_import_from_memory(
      const std::map<std::string, std::vector<DailyLog>>& data_map) override;
  void run_ingest(const std::string& source_path, DateCheckMode date_check_mode,
                  bool save_processed = false) override;
  const AppConfig& get_config() const override;

 private:
  const AppConfig& app_config_;
  std::string db_path_;
  fs::path output_root_path_;
};

#endif  // APPLICATION_WORKFLOW_HANDLER_H_
