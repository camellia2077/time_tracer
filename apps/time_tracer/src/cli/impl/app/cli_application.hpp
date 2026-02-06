// cli/impl/app/cli_application.hpp
#ifndef CLI_IMPL_APP_CLI_APPLICATION_H_
#define CLI_IMPL_APP_CLI_APPLICATION_H_

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "cli/framework/core/command_parser.hpp"
#include "cli/impl/app/app_context.hpp"  // 包含 AppContext 定义
#include "common/config/app_config.hpp"

class DBManager;
// 移除 WorkflowHandler 和 ReportHandler 的前向声明，因为成员变量已经移除了

namespace fs = std::filesystem;

class CliApplication {
 public:
  explicit CliApplication(const std::vector<std::string>& args);
  ~CliApplication();

  [[nodiscard]] auto Execute() -> int;

 private:
  CommandParser parser_;
  AppConfig app_config_;

  // --- 服务容器 ---
  // AppContext 现在负责持有核心逻辑服务 (Workflow, Report)
  std::shared_ptr<AppContext> app_context_;

  // --- 基础设施 ---
  // DBManager 属于基础设施，CliApp 仍需负责初始化
  // (注：如果 DBManager 未来也需要注入到 Context，也可以改为 shared_ptr 并放入
  // Context)
  std::unique_ptr<DBManager> db_manager_;

  fs::path output_root_path_;
  fs::path exported_files_path_;

  void InitializeOutputPaths();
};

#endif  // CLI_IMPL_APP_CLI_APPLICATION_H_
