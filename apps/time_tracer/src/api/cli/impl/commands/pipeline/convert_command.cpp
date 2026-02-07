// api/cli/impl/commands/pipeline/convert_command.cpp
#include "api/cli/impl/commands/pipeline/convert_command.hpp"

#include <memory>
#include <stdexcept>

#include "api/cli/framework/core/command_parser.hpp"
#include "api/cli/framework/core/command_registry.hpp"
#include "api/cli/framework/core/command_validator.hpp"  // [新增]
#include "api/cli/impl/app/app_context.hpp"
#include "domain/types/app_options.hpp"

static CommandRegistrar<AppContext> registrar(
    "convert", [](AppContext& ctx) -> std::unique_ptr<ConvertCommand> {
      if (!ctx.workflow_handler) {
        throw std::runtime_error("WorkflowHandler not initialized");
      }
      DateCheckMode default_date_check = ctx.config.default_date_check_mode;
      if (ctx.config.command_defaults.convert_date_check_mode) {
        default_date_check =
            *ctx.config.command_defaults.convert_date_check_mode;
      }

      bool default_save_processed = ctx.config.default_save_processed_output;
      if (ctx.config.command_defaults.convert_save_processed_output) {
        default_save_processed =
            *ctx.config.command_defaults.convert_save_processed_output;
      }

      bool default_validate_logic =
          ctx.config.command_defaults.convert_validate_logic.value_or(true);
      bool default_validate_structure =
          ctx.config.command_defaults.convert_validate_structure.value_or(true);

      return std::make_unique<ConvertCommand>(
          *ctx.workflow_handler, default_date_check, default_save_processed,
          default_validate_logic, default_validate_structure);
    });

ConvertCommand::ConvertCommand(IWorkflowHandler& workflow_handler,
                               DateCheckMode default_date_check_mode,
                               bool default_save_processed_output,
                               bool default_validate_logic,
                               bool default_validate_structure)
    : workflow_handler_(workflow_handler),
      default_date_check_mode_(default_date_check_mode),
      default_save_processed_output_(default_save_processed_output),
      default_validate_logic_(default_validate_logic),
      default_validate_structure_(default_validate_structure) {}

auto ConvertCommand::GetDefinitions() const -> std::vector<ArgDef> {
  return {{"path", ArgType::kPositional, {}, "Source file path", true, "", 0}};
}
auto ConvertCommand::GetHelp() const -> std::string {
  return "Converts source files (e.g., .txt) to processed JSON format.";
}

void ConvertCommand::Execute(const CommandParser& parser) {
  // 1. 统一验证
  ParsedArgs args = CommandValidator::Validate(parser, GetDefinitions());

  // 2. 配置选项
  AppOptions options;
  options.validate_structure = default_validate_structure_;
  options.convert = true;
  options.save_processed_output = default_save_processed_output_;
  options.validate_logic = default_validate_logic_;
  options.date_check_mode = default_date_check_mode_;

  // 3. 执行
  workflow_handler_.RunConverter(args.Get("path"), options);
}
