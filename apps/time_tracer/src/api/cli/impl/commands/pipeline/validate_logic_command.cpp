// api/cli/impl/commands/pipeline/validate_logic_command.cpp
#include "api/cli/impl/commands/pipeline/validate_logic_command.hpp"

#include <filesystem>
#include <utility>

#include "api/cli/framework/core/command_parser.hpp"
#include "api/cli/framework/core/command_registry.hpp"
#include "api/cli/framework/core/command_validator.hpp"
#include "api/cli/impl/app/app_context.hpp"
#include "api/cli/impl/utils/arg_utils.hpp"
#include "application/pipeline/pipeline_manager.hpp"
#include "domain/types/app_options.hpp"

// 注册命令：validate-logic
static CommandRegistrar<AppContext> registrar(
    "validate-logic",
    [](AppContext& ctx) -> std::unique_ptr<ValidateLogicCommand> {
      DateCheckMode default_date_check = ctx.config.default_date_check_mode;
      if (ctx.config.command_defaults.validate_logic_date_check_mode) {
        default_date_check =
            *ctx.config.command_defaults.validate_logic_date_check_mode;
      }

      std::filesystem::path output_root =
          ctx.config.defaults.output_root.value_or(
              ctx.config.export_path.value_or("./"));

      return std::make_unique<ValidateLogicCommand>(ctx.config, output_root,
                                                    default_date_check);
    });

ValidateLogicCommand::ValidateLogicCommand(
    const AppConfig& config, std::filesystem::path output_root,
    DateCheckMode default_date_check_mode)
    : app_config_(config),
      output_root_(std::move(output_root)),
      default_date_check_mode_(default_date_check_mode) {}

auto ValidateLogicCommand::GetDefinitions() const -> std::vector<ArgDef> {
  return {
      {"path", ArgType::kPositional, {}, "Source directory path", true, "", 0},
      {"date_check",
       ArgType::kOption,
       {"--date-check"},
       "Date check mode (none/continuity/full)",
       false,
       ""},
      {"no_date_check",
       ArgType::kFlag,
       {"--no-date-check"},
       "Disable date check",
       false,
       ""}};
}

auto ValidateLogicCommand::GetHelp() const -> std::string {
  return "Validates business logic (e.g., date continuity, sleep cycles). "
         "Requires conversion but won't save output.";
}

void ValidateLogicCommand::Execute(const CommandParser& parser) {
  auto args = CommandValidator::Validate(parser, GetDefinitions());

  AppOptions options;
  options.input_path = args.Get("path");

  // [关键] 逻辑验证需要先转换数据(convert)，但不保存(save=false)
  options.validate_structure =
      false;  // 可选：是否同时也做结构验证？通常可以跳过以加快速度，或者开启以保安全
  options.convert = true;                 // 必须开启，否则没有数据来验证逻辑
  options.validate_logic = true;          // 开启本功能
  options.save_processed_output = false;  // 只验证，不生成文件
  options.date_check_mode = default_date_check_mode_;

  // 解析日期检查模式
  if (args.Has("date_check")) {
    options.date_check_mode =
        ArgUtils::ParseDateCheckMode(args.Get("date_check"));
  } else if (args.Has("no_date_check")) {
    options.date_check_mode = DateCheckMode::kNone;
  }

  core::pipeline::PipelineManager manager(app_config_, output_root_);
  (void)manager.Run(options.input_path.string(), options);
}
