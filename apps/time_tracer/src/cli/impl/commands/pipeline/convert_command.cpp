// cli/impl/commands/pipeline/convert_command.cpp
#include "convert_command.hpp"

#include <memory>
#include <stdexcept>

#include "cli/framework/core/command_parser.hpp"
#include "cli/framework/core/command_registry.hpp"
#include "cli/framework/core/command_validator.hpp"  // [新增]
#include "cli/impl/app/app_context.hpp"
#include "common/app_options.hpp"

static CommandRegistrar<AppContext> registrar(
    "convert", [](AppContext& ctx) -> std::unique_ptr<ConvertCommand> {
      if (!ctx.workflow_handler) {
        throw std::runtime_error("WorkflowHandler not initialized");
      }
      return std::make_unique<ConvertCommand>(*ctx.workflow_handler);
    });

ConvertCommand::ConvertCommand(IWorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

auto ConvertCommand::get_definitions() const -> std::vector<ArgDef> {
  return {{"path", ArgType::Positional, {}, "Source file path", true, "", 0}};
}
auto ConvertCommand::get_help() const -> std::string {
  return "Converts source files (e.g., .txt) to processed JSON format.";
}

void ConvertCommand::execute(const CommandParser& parser) {
  // 1. 统一验证
  ParsedArgs args = CommandValidator::validate(parser, get_definitions());

  // 2. 配置选项
  AppOptions options;
  options.validate_structure = true;
  options.convert = true;
  options.save_processed_output = true;
  options.validate_logic = true;
  options.date_check_mode = DateCheckMode::Continuity;

  // 3. 执行
  workflow_handler_.run_converter(args.get("path"), options);
}
