// cli/impl/commands/pipeline/ingest_command.cpp
#include "cli/impl/commands/pipeline/ingest_command.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>

#include "cli/framework/core/command_parser.hpp"
#include "cli/framework/core/command_registry.hpp"
#include "cli/framework/core/command_validator.hpp"  // [新增]
#include "cli/impl/app/app_context.hpp"
#include "cli/impl/utils/arg_utils.hpp"  // [新增] 用于 DateCheckMode 解析
#include "common/app_options.hpp"

[[maybe_unused]] static CommandRegistrar<AppContext> registrar(
    "ingest", [](AppContext& ctx) -> std::unique_ptr<IngestCommand> {
      if (!ctx.workflow_handler) {
        throw std::runtime_error("WorkflowHandler not initialized");
      }
      DateCheckMode default_date_check = ctx.config.default_date_check_mode;
      if (ctx.config.command_defaults.ingest_date_check_mode) {
        default_date_check =
            *ctx.config.command_defaults.ingest_date_check_mode;
      }

      bool default_save_processed = ctx.config.default_save_processed_output;
      if (ctx.config.command_defaults.ingest_save_processed_output) {
        default_save_processed =
            *ctx.config.command_defaults.ingest_save_processed_output;
      }

      return std::make_unique<IngestCommand>(
          *ctx.workflow_handler, default_date_check, default_save_processed);
    });

IngestCommand::IngestCommand(IWorkflowHandler& workflow_handler,
                             DateCheckMode default_date_check_mode,
                             bool default_save_processed_output)
    : workflow_handler_(workflow_handler),
      default_date_check_mode_(default_date_check_mode),
      default_save_processed_output_(default_save_processed_output) {}

auto IngestCommand::GetDefinitions() const -> std::vector<ArgDef> {
  return {{"path", ArgType::kPositional, {}, "Source directory", true, "", 0},
          {"date_check",
           ArgType::kOption,
           {"--date-check"},
           "Date check mode",
           false,
           ""},
          {"no_date_check",
           ArgType::kFlag,
           {"--no-date-check"},
           "Disable date check",
           false,
           ""},
          {"save",
           ArgType::kFlag,
           {"--save-processed"},
           "Save processed JSON",
           false,
           ""},
          {"no_save",
           ArgType::kFlag,
           {"--no-save"},
           "Do not save processed JSON",
           false,
           ""}};
}
auto IngestCommand::GetHelp() const -> std::string {
  return "Runs the full data ingestion pipeline: validate source -> convert -> "
         "validate logic -> import.";
}

void IngestCommand::Execute(const CommandParser& parser) {
  // 1. 统一验证与提取
  ParsedArgs args = CommandValidator::Validate(parser, GetDefinitions());

  const std::string kInputPath = args.Get("path");

  // 2. 解析逻辑参数
  DateCheckMode mode = default_date_check_mode_;

  if (args.Has("date_check")) {
    mode = ArgUtils::ParseDateCheckMode(args.Get("date_check"));
  } else if (args.Has("no_date_check")) {
    mode = DateCheckMode::kNone;
  }

  bool save_json = default_save_processed_output_;
  if (args.Has("no_save")) {
    save_json = false;
  } else if (args.Has("save")) {
    save_json = true;
  }

  // 3. 执行业务
  workflow_handler_.RunIngest(kInputPath, mode, save_json);
}
