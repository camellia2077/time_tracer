// application/interfaces/i_workflow_handler.hpp
#ifndef APPLICATION_INTERFACES_I_WORKFLOW_HANDLER_H_
#define APPLICATION_INTERFACES_I_WORKFLOW_HANDLER_H_

#include <map>
#include <string>
#include <vector>

#include "domain/logic/validator/common/validator_utils.hpp"
#include "domain/model/daily_log.hpp"
#include "domain/types/app_options.hpp"

// 前向声明，减少头文件依赖
struct AppConfig;

class IWorkflowHandler {
 public:
  virtual ~IWorkflowHandler() = default;

  // 运行预处理流水线
  virtual auto RunConverter(const std::string& input_path,
                            const AppOptions& options) -> void = 0;

  // 传统的基于文件的导入
  virtual auto RunDatabaseImport(const std::string& processed_path_str)
      -> void = 0;

  // 基于内存数据的导入
  virtual auto RunDatabaseImportFromMemory(
      const std::map<std::string, std::vector<DailyLog>>& data_map) -> void = 0;

  // 完整流程 (Ingest/Blink)
  virtual auto RunIngest(const std::string& source_path,
                         DateCheckMode date_check_mode,
                         bool save_processed = false) -> void = 0;

  // 获取配置
  [[nodiscard]] virtual auto GetConfig() const -> const AppConfig& = 0;
};

#endif  // APPLICATION_INTERFACES_I_WORKFLOW_HANDLER_H_
