// api/cli/impl/app/cli_application.cpp
#include "api/cli/impl/app/cli_application.hpp"

#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "api/cli/framework/core/command_registry.hpp"
#include "api/cli/framework/interfaces/i_command.hpp"
#include "api/cli/impl/commands/register_all_commands.hpp"
#include "api/cli/impl/utils/console_helper.hpp"
#include "application/ports/i_cli_runtime_factory.hpp"
#include "shared/types/ansi_colors.hpp"
#include "shared/types/exceptions.hpp"
#include "shared/types/exit_codes.hpp"

namespace fs = std::filesystem;

namespace {

auto IsCoreManagedPipelineCommand(const std::string &command_name) -> bool {
  return command_name == "convert" || command_name == "ingest" ||
         command_name == "validate-logic" ||
         command_name == "validate-structure";
}

auto PrintErrorAndReturn(AppExitCode exit_code, std::string_view prefix,
                         std::string_view message) -> int {
  namespace colors = tracer_core::common::colors;
  std::cerr << colors::kRed << prefix << message << colors::kReset << std::endl;
  return static_cast<int>(exit_code);
}

} // namespace

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
CliApplication::CliApplication(
    const std::vector<std::string> &args,
    std::shared_ptr<tracer_core::application::ports::ICliRuntimeFactory>
        runtime_factory)
    : parser_(args, GetAppParserConfig()),
      app_context_(std::make_shared<AppContext>()),
      runtime_factory_(std::move(runtime_factory)) {
  tracer_core::cli::impl::commands::RegisterAllCommands();
  if (!runtime_factory_) {
    throw std::runtime_error("CLI runtime factory not initialized.");
  }
}

CliApplication::~CliApplication() = default;

void CliApplication::BuildRuntime() {
  tracer_core::application::ports::CliRuntimeRequest request;
  request.executable_path = fs::path(parser_.GetRawArg(0));

  try {
    request.command_name = parser_.GetCommand();
  } catch (...) {
    request.command_name.clear();
  }

  if (auto user_db_path = parser_.GetOption({"--db", "--database"})) {
    request.db_override = fs::absolute(*user_db_path);
  }
  if (auto path_opt = parser_.GetOption({"-o", "--output"})) {
    request.output_override = fs::absolute(*path_opt);
  }

  auto runtime = runtime_factory_->BuildRuntime(request);
  if (!runtime.core_api) {
    throw std::runtime_error("Runtime factory returned null core API.");
  }

  app_context_->core_api = std::move(runtime.core_api);
  app_context_->config = std::move(runtime.cli_config);
  runtime_state_ = std::move(runtime.runtime_state);
  runtime_built_ = true;
}

void CliApplication::EnsureRuntimeBuilt() {
  if (runtime_built_) {
    return;
  }
  BuildRuntime();
}

auto CliApplication::Execute() -> int {
  using namespace tracer_core::common;
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
      if (command_name != "doctor") {
        EnsureRuntimeBuilt();
      }
      auto command = CommandRegistry<AppContext>::Instance().CreateCommand(
          command_name, *app_context_);
      if (command) {
        ConsoleHelper::PrintCommandUsage(command_name, *command);

        return static_cast<int>(AppExitCode::kSuccess);
      }
    }

    EnsureRuntimeBuilt();
    auto commands = CommandRegistry<AppContext>::Instance().CreateAllCommands(
        *app_context_);
    ConsoleHelper::PrintFullUsage(parser_.GetRawArg(0).c_str(), commands);

    return static_cast<int>(AppExitCode::kSuccess);
  }

  if (command_name != "doctor") {
    EnsureRuntimeBuilt();
  }

  auto command = CommandRegistry<AppContext>::Instance().CreateCommand(
      command_name, *app_context_);

  if (command) {
    try {
      command->Execute(parser_);
    } catch (const DatabaseError &e) {
      return PrintErrorAndReturn(AppExitCode::kDatabaseError,
                                 "Database Error: ", e.what());
    } catch (const IoError &e) {
      return PrintErrorAndReturn(AppExitCode::kIoError,
                                 "I/O Error: ", e.what());
    } catch (const ConfigError &e) {
      return PrintErrorAndReturn(AppExitCode::kConfigError,
                                 "Configuration Error: ", e.what());
    } catch (const DllCompatibilityError &e) {
      return PrintErrorAndReturn(AppExitCode::kDllCompatibilityError,
                                 "DLL Compatibility Error: ", e.what());
    } catch (const LogicError &e) {
      if (!IsCoreManagedPipelineCommand(command_name)) {
        return PrintErrorAndReturn(AppExitCode::kLogicError,
                                   "Logic Error: ", e.what());
      }
      return static_cast<int>(AppExitCode::kLogicError);
    } catch (const std::invalid_argument &e) {
      return PrintErrorAndReturn(AppExitCode::kInvalidArguments,
                                 "Invalid Argument: ", e.what());
    } catch (const std::exception &e) {
      return PrintErrorAndReturn(AppExitCode::kGenericError,
                                 std::string("Error executing command '") +
                                     command_name + "': ",
                                 e.what());
    } catch (...) {
      return PrintErrorAndReturn(AppExitCode::kUnknownError, "",
                                 "Unknown error occurred.");
    }
  } else {
    namespace colors = tracer_core::common::colors;
    std::cerr << colors::kRed << "Unknown command '" << command_name << "'."
              << colors::kReset << std::endl;
    std::cerr << colors::kGray << "Run with --help to see available commands."
              << colors::kReset << std::endl;
    return static_cast<int>(AppExitCode::kCommandNotFound);
  }

  return static_cast<int>(AppExitCode::kSuccess);
}
