// cli/impl/commands/pipeline/ingest_command.cpp
#include "ingest_command.hpp"

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
      return std::make_unique<IngestCommand>(*ctx.workflow_handler);
    });

IngestCommand::IngestCommand(IWorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

auto IngestCommand::get_definitions() const -> std::vector<ArgDef> {
  return {{"path", ArgType::Positional, {}, "Source directory", true, "", 0},
          {"date_check",
           ArgType::Option,
           {"--date-check"},
           "Date check mode",
           false,
           ""},
          {"no_date_check",
           ArgType::Flag,
           {"--no-date-check"},
           "Disable date check",
           false,
           ""},
          {"save",
           ArgType::Flag,
           {"--save-processed"},
           "Save processed JSON",
           false,
           ""},
          {"no_save",
           ArgType::Flag,
           {"--no-save"},
           "Do not save processed JSON",
           false,
           ""}};
}
auto IngestCommand::get_help() const -> std::string {
  return "Runs the full data ingestion pipeline: validate source -> convert -> "
         "validate logic -> import.";
}

void IngestCommand::execute(const CommandParser& parser) {
  // 1. 统一验证与提取
  ParsedArgs args = CommandValidator::validate(parser, get_definitions());

  const std::string kInputPath = args.get("path");

  // 2. 解析逻辑参数
  DateCheckMode mode = DateCheckMode::Continuity;

  if (args.has("date_check")) {
    mode = ArgUtils::parse_date_check_mode(args.get("date_check"));
  } else if (args.has("no_date_check")) {
    mode = DateCheckMode::None;
  }

  bool save_json = true;
  if (args.has("no_save")) {
    save_json = false;
  } else if (args.has("save")) {
    save_json = true;
  }

  // 3. 执行业务
  workflow_handler_.run_ingest(kInputPath, mode, save_json);
}
