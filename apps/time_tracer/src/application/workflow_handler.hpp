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
#include "validator/common/validator_utils.hpp"

namespace fs = std::filesystem;

// [修改] 继承 IWorkflowHandler
class WorkflowHandler : public IWorkflowHandler {
 public:
  WorkflowHandler(std::string db_path, const AppConfig& config,
                  fs::path output_root_path);
  ~WorkflowHandler() override;

  auto RunConverter(const std::string& input_path, const AppOptions& options)
      -> void override;
  auto RunDatabaseImport(const std::string& processed_path_str)
      -> void override;
  auto RunDatabaseImportFromMemory(
      const std::map<std::string, std::vector<DailyLog>>& data_map)
      -> void override;
  auto RunIngest(const std::string& source_path, DateCheckMode date_check_mode,
                 bool save_processed = false) -> void override;
  [[nodiscard]] auto GetConfig() const -> const AppConfig& override;

 private:
  const AppConfig& app_config_;
  std::string db_path_;
  fs::path output_root_path_;
};

#endif  // APPLICATION_WORKFLOW_HANDLER_H_
