// api/cli/impl/app/cli_application.cpp
#include "api/cli/impl/app/cli_application.hpp"

#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "api/cli/framework/core/command_registry.hpp"
#include "api/cli/framework/interfaces/i_command.hpp"
#include "api/cli/impl/utils/help_formatter.hpp"
#include "application/reporting/generator/report_generator.hpp"
#include "application/reporting/report_handler.hpp"
#include "application/workflow_handler.hpp"
#include "infrastructure/config/config_loader.hpp"
#include "infrastructure/io/file_controller.hpp"
#include "infrastructure/persistence/repositories/sqlite_project_repository.hpp"
#include "infrastructure/persistence/sqlite/db_manager.hpp"
#include "infrastructure/reports/exporter.hpp"
#include "infrastructure/reports/formatter_registry.hpp"
#include "infrastructure/reports/report_service.hpp"
#include "shared/types/ansi_colors.hpp"
#include "shared/types/exceptions.hpp"
#include "shared/types/exit_codes.hpp"

namespace fs = std::filesystem;
const std::string kDatabaseFilename = "time_data.sqlite3";

// [新增] 辅助函数，定义应用特定的全局参数规则
static auto GetAppParserConfig() -> ParserConfig {
  return {// global_value_options: 这些选项后面跟着一个值，需要从位置参数中剔除
          .global_value_options = {"-o", "--output", "-f", "--format",
                                   "--date-check", "--db", "--database"},
          // global_flag_options: 这些是布尔开关
          .global_flag_options = {"--save-processed", "--no-save",
                                  "--no-date-check"}};
}

// [修改] 在初始化列表中调用 get_app_parser_config()
CliApplication::CliApplication(const std::vector<std::string>& args)
    : parser_(args, GetAppParserConfig()) {
  // 0. 初始化服务容器
  app_context_ = std::make_shared<AppContext>();

  // 1. 路径初始化
  fs::path exe_path(parser_.GetRawArg(0));
  ConfigLoader config_loader(parser_.GetRawArg(0));
  app_config_ = config_loader.LoadConfiguration();
  app_context_->config = app_config_;

  fs::path default_output_root =
      fs::absolute(exe_path.parent_path() / "output");
  if (app_config_.defaults.output_root) {
    default_output_root = fs::absolute(*app_config_.defaults.output_root);
  }

  fs::path default_db_path = default_output_root / "db" / kDatabaseFilename;
  if (app_config_.defaults.db_path) {
    default_db_path = fs::absolute(*app_config_.defaults.db_path);
  }

  fs::path default_reports_root = default_output_root / "reports";

  fs::path db_path;
  if (auto user_db_path = parser_.GetOption({"--db", "--database"})) {
    db_path = fs::absolute(*user_db_path);
  } else {
    db_path = default_db_path;
  }
  app_context_->db_path = db_path;

  if (auto path_opt = parser_.GetOption({"-o", "--output"})) {
    exported_files_path_ = fs::absolute(*path_opt);
  } else {
    exported_files_path_ = default_reports_root;
  }
  output_root_path_ = default_output_root;

  // 2. 初始化基础设施
  FileController::PrepareOutputDirectories(output_root_path_,
                                           exported_files_path_);

  db_manager_ = std::make_unique<DBManager>(db_path.string());

  // 3. 初始化并注入核心服务到 Context
  // [WorkflowHandler]
  auto workflow_impl = std::make_shared<WorkflowHandler>(
      db_path.string(), app_config_, output_root_path_);
  app_context_->workflow_handler = workflow_impl;  // 注入接口

  // 数据库连接检查 (逻辑保持不变)
  const std::string kCommand = parser_.GetCommand();
  if (kCommand == "query" || kCommand == "export") {
    if (!db_manager_->OpenDatabaseIfNeeded()) {
      throw std::runtime_error(
          "Failed to open database at: " + db_path.string() +
          "\nPlease ensure data has been imported or check the path.");
    }
  }
  sqlite3* db_connection = db_manager_->GetDbConnection();

  // [ReportHandler]
  reports::RegisterReportFormatters();
  auto report_query_service =
      std::make_unique<ReportService>(db_connection, app_config_);
  auto report_generator =
      std::make_unique<ReportGenerator>(std::move(report_query_service));
  std::unique_ptr<IReportExporter> exporter =
      std::make_unique<Exporter>(exported_files_path_);

  auto report_impl = std::make_shared<ReportHandler>(
      std::move(report_generator), std::move(exporter));
  app_context_->report_handler = report_impl;  // 注入接口

  // [Repository]
  app_context_->project_repository =
      std::make_shared<SqliteProjectRepository>(db_path.string());
}

CliApplication::~CliApplication() = default;

auto CliApplication::Execute() -> int {
  using namespace time_tracer::common;
  // [新增] 处理 Help 全局标记
  bool request_help = parser_.HasFlag({"--help", "-h"});
  std::string command_name;
  try {
    command_name = parser_.GetCommand();
  } catch (...) {
    request_help = true;
  }

  if (request_help) {
    // [修改] 如果指定了具体的命令，只显示该命令的帮助
    if (!command_name.empty()) {
      auto command = CommandRegistry<AppContext>::Instance().CreateCommand(
          command_name, *app_context_);
      if (command) {
        PrintCommandUsage(command_name, *command);

        return static_cast<int>(AppExitCode::kSuccess);
      }
    }

    auto commands = CommandRegistry<AppContext>::Instance().CreateAllCommands(
        *app_context_);
    PrintFullUsage(parser_.GetRawArg(0).c_str(), commands);

    return static_cast<int>(AppExitCode::kSuccess);
  }

  auto command = CommandRegistry<AppContext>::Instance().CreateCommand(
      command_name, *app_context_);

  if (command) {
    try {
      command->Execute(parser_);
    } catch (const DatabaseError& e) {
      namespace colors = time_tracer::common::colors;
      std::cerr << colors::kRed << "Database Error: " << e.what()
                << colors::kReset << std::endl;
      return static_cast<int>(AppExitCode::kDatabaseError);
    } catch (const IoError& e) {
      namespace colors = time_tracer::common::colors;
      std::cerr << colors::kRed << "I/O Error: " << e.what() << colors::kReset
                << std::endl;
      return static_cast<int>(AppExitCode::kIoError);
    } catch (const ConfigError& e) {
      namespace colors = time_tracer::common::colors;
      std::cerr << colors::kRed << "Configuration Error: " << e.what()
                << colors::kReset << std::endl;
      return static_cast<int>(AppExitCode::kConfigError);
    } catch (const LogicError& e) {
      namespace colors = time_tracer::common::colors;
      std::cerr << colors::kRed << "Logic Error: " << e.what() << colors::kReset
                << std::endl;
      return static_cast<int>(AppExitCode::kLogicError);
    } catch (const std::invalid_argument& e) {
      namespace colors = time_tracer::common::colors;
      std::cerr << colors::kRed << "Invalid Argument: " << e.what()
                << colors::kReset << std::endl;
      return static_cast<int>(AppExitCode::kInvalidArguments);
    } catch (const std::exception& e) {
      namespace colors = time_tracer::common::colors;
      std::cerr << colors::kRed << "Error executing command '" << command_name
                << "': " << e.what() << colors::kReset << std::endl;
      return static_cast<int>(AppExitCode::kGenericError);
    } catch (...) {
      namespace colors = time_tracer::common::colors;
      std::cerr << colors::kRed << "Unknown error occurred." << colors::kReset
                << std::endl;
      return static_cast<int>(AppExitCode::kUnknownError);
    }
  } else {
    namespace colors = time_tracer::common::colors;
    std::cerr << colors::kRed << "Unknown command '" << command_name << "'."
              << colors::kReset << std::endl;
    std::cout << "Run with --help to see available commands." << std::endl;
    return static_cast<int>(AppExitCode::kCommandNotFound);
  }

  return static_cast<int>(AppExitCode::kSuccess);
}

void CliApplication::InitializeOutputPaths() {}
