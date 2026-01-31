// cli/impl/commands/pipeline/validate_logic_command.cpp
#include "validate_logic_command.hpp"

#include <utility>

#include "cli/framework/core/command_parser.hpp"
#include "cli/framework/core/command_registry.hpp"
#include "cli/framework/core/command_validator.hpp"
#include "cli/impl/app/app_context.hpp"
#include "cli/impl/utils/arg_utils.hpp"
#include "common/app_options.hpp"
#include "application/pipeline/pipeline_manager.hpp"

// 注册命令：validate-logic
static CommandRegistrar<AppContext> registrar(
    "validate-logic",
    [](AppContext& ctx) -> std::unique_ptr<ValidateLogicCommand> {
      return std::make_unique<ValidateLogicCommand>(
          ctx.config, ctx.config.export_path.value_or("./"));
    });

ValidateLogicCommand::ValidateLogicCommand(const AppConfig& config,
                                           std::filesystem::path output_root)
    : app_config_(config), output_root_(std::move(output_root)) {}

auto ValidateLogicCommand::get_definitions() const -> std::vector<ArgDef> {
  return {
      {"path", ArgType::Positional, {}, "Source directory path", true, "", 0},
      {"date_check",
       ArgType::Option,
       {"--date-check"},
       "Date check mode (none/continuity/full)",
       false,
       "none"},
      {"no_date_check",
       ArgType::Flag,
       {"--no-date-check"},
       "Disable date check",
       false,
       ""}};
}

auto ValidateLogicCommand::get_help() const -> std::string {
  return "Validates business logic (e.g., date continuity, sleep cycles). "
         "Requires conversion but won't save output.";
}

void ValidateLogicCommand::execute(const CommandParser& parser) {
  auto args = CommandValidator::validate(parser, get_definitions());

  AppOptions options;
  options.input_path = args.get("path");

  // [关键] 逻辑验证需要先转换数据(convert)，但不保存(save=false)
  options.validate_structure =
      false;  // 可选：是否同时也做结构验证？通常可以跳过以加快速度，或者开启以保安全
  options.convert = true;                 // 必须开启，否则没有数据来验证逻辑
  options.validate_logic = true;          // 开启本功能
  options.save_processed_output = false;  // 只验证，不生成文件

  // 解析日期检查模式
  if (args.has("date_check")) {
    options.date_check_mode =
        ArgUtils::parse_date_check_mode(args.get("date_check"));
  } else if (args.has("no_date_check")) {
    options.date_check_mode = DateCheckMode::None;
  }

  core::pipeline::PipelineManager manager(app_config_, output_root_);
  manager.run(options.input_path.string(), options);
}
