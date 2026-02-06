// cli/impl/commands/pipeline/validate_structure_command.cpp
#include "cli/impl/commands/pipeline/validate_structure_command.hpp"

#include <filesystem>
#include <iostream>
#include <utility>

#include "application/pipeline/pipeline_manager.hpp"  // 直接调用 Pipeline
#include "cli/framework/core/command_parser.hpp"
#include "cli/framework/core/command_registry.hpp"
#include "cli/framework/core/command_validator.hpp"
#include "cli/impl/app/app_context.hpp"
#include "common/app_options.hpp"

// 注册命令：validate-structure
static CommandRegistrar<AppContext> registrar(
    "validate-structure",
    [](AppContext& ctx) -> std::unique_ptr<ValidateStructureCommand> {
      std::filesystem::path output_root =
          ctx.config.defaults.output_root.value_or(
              ctx.config.export_path.value_or("./"));
      return std::make_unique<ValidateStructureCommand>(ctx.config,
                                                        output_root);
    });

ValidateStructureCommand::ValidateStructureCommand(
    const AppConfig& config, std::filesystem::path output_root)
    : app_config_(config), output_root_(std::move(output_root)) {}

auto ValidateStructureCommand::GetDefinitions() const -> std::vector<ArgDef> {
  return {{"path",
           ArgType::kPositional,
           {},
           "Source directory or file path",
           true,
           "",
           0}};
}

auto ValidateStructureCommand::GetHelp() const -> std::string {
  return "Validates the syntax and structure of source TXT files (Read-only).";
}

void ValidateStructureCommand::Execute(const CommandParser& parser) {
  auto args = CommandValidator::Validate(parser, GetDefinitions());

  AppOptions options;
  options.input_path = args.Get("path");

  // [关键] 只开启结构验证
  options.validate_structure = true;
  options.convert = false;
  options.validate_logic = false;
  options.save_processed_output = false;

  // 委托给 Core 执行
  core::pipeline::PipelineManager manager(app_config_, output_root_);
  (void)manager.Run(options.input_path.string(), options);
}
